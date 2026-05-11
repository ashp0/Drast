#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Codegen.h"
#include "Lexer.h"
#include "Parser.h"
#include "Token.h"

namespace fs = std::filesystem;

static std::string readFile(const fs::path &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open '" + path.string() + "'");
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static int usage() {
    std::cerr << "usage: drastc [--path dir] [--no-runtime] <input.drast>... [-o output.cpp]\n";
    return 1;
}

static fs::path canonicalExistingPath(const fs::path &path) {
    std::error_code ec;
    fs::path canonical = fs::canonical(path, ec);
    if (!ec) return canonical;
    return fs::absolute(path).lexically_normal();
}

static std::string dottedModulePath(std::string path) {
    std::replace(path.begin(), path.end(), '.', fs::path::preferred_separator);
    return path;
}

static void appendProgram(drast::Program &target, drast::Program source) {
    for (auto &use : source.uses) target.uses.push_back(std::move(use));
    for (auto &global : source.globals) target.globals.push_back(std::move(global));
    for (auto &fn : source.functions) target.functions.push_back(std::move(fn));
    for (auto &s : source.structs) target.structs.push_back(std::move(s));
    for (auto &e : source.enums) target.enums.push_back(std::move(e));
    for (auto &p : source.protocols) target.protocols.push_back(std::move(p));
    for (auto &impl : source.impls) target.impls.push_back(std::move(impl));
}

class ModuleLoader {
public:
    explicit ModuleLoader(std::vector<fs::path> searchPaths)
        : searchPaths_(std::move(searchPaths)) {}

    drast::Program loadAll(const std::vector<fs::path> &roots) {
        for (const fs::path &root : roots) {
            loadFile(root);
        }
        drast::Program merged;
        for (auto &entry : ordered_) {
            appendProgram(merged, std::move(entry.second));
        }
        return merged;
    }

private:
    enum class State { Visiting, Done };

    std::vector<fs::path> searchPaths_;
    std::unordered_map<std::string, State> states_;
    std::vector<std::pair<std::string, drast::Program>> ordered_;
    std::vector<std::string> stack_;

    drast::Program parseFile(const fs::path &path) const {
        std::string source = readFile(path);
        drast::Lexer lexer(source, path.string());
        std::vector<drast::Token> tokens = lexer.lex();
        drast::Parser parser(std::move(tokens));
        return parser.parse();
    }

    bool shouldTryDrastImport(const drast::UseDecl &use) const {
        if (use.path == "std") return false;
        fs::path path(use.path);
        std::string ext = path.extension().string();
        if (ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx") {
            return false;
        }
        return true;
    }

    std::optional<fs::path> existingCandidate(const fs::path &candidate) const {
        std::error_code ec;
        if (fs::exists(candidate, ec) && fs::is_regular_file(candidate, ec)) {
            return canonicalExistingPath(candidate);
        }
        return std::nullopt;
    }

    std::vector<fs::path> importBases(const fs::path &fromDir) const {
        std::vector<fs::path> bases;
        bases.push_back(fromDir);
        for (const fs::path &path : searchPaths_) bases.push_back(path);
        return bases;
    }

    std::optional<fs::path> resolveImport(const drast::UseDecl &use,
                                         const fs::path &fromDir) const {
        if (!shouldTryDrastImport(use)) return std::nullopt;

        std::vector<fs::path> names;
        fs::path raw(use.path);
        names.push_back(raw);
        if (raw.extension() != ".drast") {
            names.push_back(raw.string() + ".drast");
            names.push_back(fs::path(dottedModulePath(use.path) + ".drast"));
        }

        for (const fs::path &base : importBases(fromDir)) {
            for (const fs::path &name : names) {
                fs::path candidate = name.is_absolute() ? name : base / name;
                if (auto found = existingCandidate(candidate)) return found;
            }
        }
        return std::nullopt;
    }

    void loadFile(const fs::path &requestedPath) {
        fs::path path = canonicalExistingPath(requestedPath);
        std::string key = path.string();

        auto stateIt = states_.find(key);
        if (stateIt != states_.end()) {
            if (stateIt->second == State::Visiting) {
                std::ostringstream msg;
                msg << "circular import detected:";
                for (const std::string &item : stack_) msg << " " << item << " ->";
                msg << " " << key;
                throw std::runtime_error(msg.str());
            }
            return;
        }

        states_[key] = State::Visiting;
        stack_.push_back(key);

        drast::Program program = parseFile(path);
        fs::path fromDir = path.parent_path();
        for (drast::UseDecl &use : program.uses) {
            if (auto dependency = resolveImport(use, fromDir)) {
                use.isDrastModule = true;
                loadFile(*dependency);
            }
        }

        stack_.pop_back();
        states_[key] = State::Done;
        ordered_.push_back({key, std::move(program)});
    }
};

int main(int argc, char **argv) {
    std::vector<fs::path> inputs;
    std::vector<fs::path> searchPaths;
    std::string output;
    bool noRuntime = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 >= argc) return usage();
            output = argv[++i];
        } else if (arg == "--path") {
            if (i + 1 >= argc) return usage();
            searchPaths.push_back(canonicalExistingPath(argv[++i]));
        } else if (arg == "--no-runtime") {
            noRuntime = true;
        } else if (!arg.empty() && arg[0] == '-') {
            return usage();
        } else {
            inputs.emplace_back(arg);
        }
    }
    if (inputs.empty()) return usage();

    try {
        ModuleLoader loader(std::move(searchPaths));
        drast::Program program = loader.loadAll(inputs);
        for (const drast::UseDecl &use : program.uses) {
            if (use.path == "no_runtime") noRuntime = true;
        }

        drast::Codegen codegen;
        codegen.setNoRuntime(noRuntime);
        std::string cpp = codegen.emit(program);

        if (output.empty()) {
            std::cout << cpp;
        } else {
            std::ofstream out(output);
            if (!out) {
                std::cerr << "drastc: cannot write '" << output << "'\n";
                return 1;
            }
            out << cpp;
        }
        return 0;
    } catch (const drast::CompileError &e) {
        std::cerr << "drastc: " << e.what() << "\n";
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "drastc: " << e.what() << "\n";
        return 1;
    }
}
