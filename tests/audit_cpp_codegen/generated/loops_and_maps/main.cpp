#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace __drt {
template <typename T> void write_one(const T& value) { std::cout << value; }
inline void write_one(const std::exception& value) { std::cout << value.what(); }
template <typename T> void write_one(const std::optional<T>& value) { if (value) write_one(*value); }
template <typename... Args> void print(const Args&... args) { (write_one(args), ...); }
template <typename... Args> void println(const Args&... args) { print(args...); std::cout << '\n'; }
inline std::string getInput(const std::string& prompt = "") { if (!prompt.empty()) std::cout << prompt; std::string line; std::getline(std::cin, line); return line; }
inline std::vector<std::string>& program_args_store() { static std::vector<std::string> values; return values; }
inline void setArgs(int argc, char** argv) { auto& values = program_args_store(); values.clear(); for (int i = 0; i < argc; ++i) values.emplace_back(argv[i] ? argv[i] : ""); }
inline const std::vector<std::string>& args() { return program_args_store(); }
inline std::string arg(std::size_t index) { const auto& values = program_args_store(); return index < values.size() ? values[index] : std::string(); }
inline bool fileExists(const std::string& path) { std::ifstream in(path); return static_cast<bool>(in); }
inline std::string readFile(const std::string& path) { std::ifstream in(path, std::ios::binary); if (!in) return ""; std::ostringstream ss; ss << in.rdbuf(); return ss.str(); }
inline bool writeFile(const std::string& path, const std::string& contents) { std::ofstream out(path, std::ios::binary); if (!out) return false; out << contents; return static_cast<bool>(out); }
inline std::string getEnv(const std::string& name) { const char* value = std::getenv(name.c_str()); return value ? std::string(value) : std::string(); }
inline std::string normalizePath(const std::string& path) { if (path.empty()) return "."; return std::filesystem::path(path).lexically_normal().string(); }
inline std::string canonicalPath(const std::string& path) { std::error_code ec; auto canonical = std::filesystem::weakly_canonical(std::filesystem::path(path), ec); if (ec) return normalizePath(path); return canonical.lexically_normal().string(); }
inline std::string currentDir() { std::error_code ec; auto cwd = std::filesystem::current_path(ec); return ec ? std::string(".") : cwd.lexically_normal().string(); }
inline bool isAbsolutePath(const std::string& path) { return std::filesystem::path(path).is_absolute(); }
inline std::string pathJoin(const std::string& left, const std::string& right) { if (left.empty()) return normalizePath(right); if (right.empty()) return normalizePath(left); return (std::filesystem::path(left) / std::filesystem::path(right)).lexically_normal().string(); }
inline std::string pathDirname(const std::string& path) { auto parent = std::filesystem::path(path).parent_path(); return parent.empty() ? std::string(".") : parent.lexically_normal().string(); }
inline std::string pathBasename(const std::string& path) { return std::filesystem::path(path).filename().string(); }
inline std::string pathStem(const std::string& path) { return std::filesystem::path(path).stem().string(); }
inline bool isDirectory(const std::string& path) { std::error_code ec; return std::filesystem::is_directory(path, ec); }
inline bool ensureDir(const std::string& path) { if (path.empty()) return false; std::error_code ec; if (std::filesystem::exists(path, ec)) return std::filesystem::is_directory(path, ec); return std::filesystem::create_directories(path, ec) || std::filesystem::is_directory(path, ec); }
inline bool removeDirRecursive(const std::string& path) { std::error_code ec; if (!std::filesystem::exists(path, ec)) return true; std::filesystem::remove_all(path, ec); return !ec; }
inline bool makePathWritable(const std::string& path) { std::error_code ec; if (!std::filesystem::exists(path, ec)) return true; std::filesystem::permissions(path, std::filesystem::perms::owner_write, std::filesystem::perm_options::add, ec); return !ec; }
inline bool makePathReadOnly(const std::string& path) { std::error_code ec; if (!std::filesystem::exists(path, ec)) return false; auto write_bits = std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write; std::filesystem::permissions(path, write_bits, std::filesystem::perm_options::remove, ec); return !ec; }
inline bool sourceNewerThanTarget(const std::string& source, const std::string& target) { std::error_code ec; if (!std::filesystem::exists(target, ec)) return true; if (!std::filesystem::exists(source, ec)) return false; auto source_time = std::filesystem::last_write_time(source, ec); if (ec) return true; auto target_time = std::filesystem::last_write_time(target, ec); if (ec) return true; return source_time > target_time; }
inline bool targetMissingOrOlder(const std::string& source, const std::string& target) { return sourceNewerThanTarget(source, target); }
inline std::string sanitizePathFragment(const std::string& text) { std::string out; for (char ch : text) { if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-' || ch == '.') out += ch; else out += '_'; } return out.empty() ? std::string("external") : out; }
inline std::string sourceOutputPath(const std::string& root, const std::string& source, const std::string& out_dir, const std::string& extension) { std::error_code ec; auto root_abs = std::filesystem::absolute(root, ec).lexically_normal(); if (ec) root_abs = std::filesystem::path(root).lexically_normal(); auto source_abs = std::filesystem::absolute(source, ec).lexically_normal(); if (ec) source_abs = std::filesystem::path(source).lexically_normal(); auto rel = std::filesystem::relative(source_abs, root_abs, ec); bool external = ec || rel.empty() || rel.is_absolute(); if (!external) { auto rel_text = rel.generic_string(); external = rel_text == ".." || rel_text.rfind("../", 0) == 0; } if (external) rel = std::filesystem::path("_external") / sanitizePathFragment(source_abs.string()); rel.replace_extension(extension); return (std::filesystem::path(out_dir) / rel).lexically_normal().string(); }
inline std::vector<std::string> sourceIncludeDirs(const std::vector<std::string>& sources) { std::set<std::string> dirs; for (const auto& source : sources) dirs.insert(pathDirname(source)); return std::vector<std::string>(dirs.begin(), dirs.end()); }
inline bool isHeaderPath(const std::string& path) { auto ext = std::filesystem::path(path).extension().string(); return ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx"; }
inline std::vector<std::string> discoverDrastSources(const std::string& root) { std::vector<std::string> out; std::error_code ec; if (!std::filesystem::exists(root, ec)) return out; std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, ec); std::filesystem::recursive_directory_iterator end; while (!ec && it != end) { const auto& entry = *it; auto name = entry.path().filename().string(); if (entry.is_directory(ec) && (name == ".drast" || name == ".git" || name == "build")) it.disable_recursion_pending(); else if (entry.is_regular_file(ec) && entry.path().extension() == ".drast") out.push_back(entry.path().lexically_normal().string()); it.increment(ec); } std::sort(out.begin(), out.end()); out.erase(std::unique(out.begin(), out.end()), out.end()); return out; }
inline std::vector<std::string> discoverDrastSourceSiblings(const std::string& root) { std::vector<std::string> out; std::error_code ec; if (!std::filesystem::exists(root, ec)) return out; std::filesystem::directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, ec); std::filesystem::directory_iterator end; while (!ec && it != end) { const auto& entry = *it; if (entry.is_regular_file(ec) && entry.path().extension() == ".drast") out.push_back(entry.path().lexically_normal().string()); it.increment(ec); } std::sort(out.begin(), out.end()); out.erase(std::unique(out.begin(), out.end()), out.end()); return out; }
inline std::string stripLineComment(const std::string& line) { bool quoted = false; char quote = '\0'; bool escaped = false; for (std::size_t i = 0; i + 1 < line.size(); ++i) { char ch = line[i]; if (escaped) { escaped = false; continue; } if (quoted && ch == '\\') { escaped = true; continue; } if (quoted) { if (ch == quote) quoted = false; continue; } if (ch == '\'' || ch == '"') { quoted = true; quote = ch; continue; } if (ch == '/' && line[i + 1] == '/') return line.substr(0, i); } return line; }
inline std::optional<std::string> parseUsePath(const std::string& line, bool* header_hint = nullptr) { if (header_hint) *header_hint = false; std::string text = line; text = text.substr(0, stripLineComment(text).size()); auto first = text.find_first_not_of(" \t\r\n"); if (first == std::string::npos) return std::nullopt; text = text.substr(first); if (text.rfind("use", 0) != 0) return std::nullopt; if (text.size() > 3 && !std::isspace(static_cast<unsigned char>(text[3]))) return std::nullopt; text = text.substr(3); first = text.find_first_not_of(" \t\r\n"); if (first == std::string::npos) return std::nullopt; text = text.substr(first); if (text.rfind("file", 0) == 0 && (text.size() == 4 || std::isspace(static_cast<unsigned char>(text[4])))) { if (header_hint) *header_hint = true; text = text.substr(4); first = text.find_first_not_of(" \t\r\n"); if (first == std::string::npos) return std::nullopt; text = text.substr(first); } if (text.empty()) return std::nullopt; if (text.front() == '\'' || text.front() == '"') { char quote = text.front(); std::string out; bool escaped = false; for (std::size_t i = 1; i < text.size(); ++i) { char ch = text[i]; if (escaped) { out += ch; escaped = false; continue; } if (ch == '\\') { escaped = true; continue; } if (ch == quote) return out; out += ch; } return out; } std::string out; for (char ch : text) { if (std::isspace(static_cast<unsigned char>(ch))) break; out += ch; } return out.empty() ? std::optional<std::string>() : out; }
inline std::string resolveDrastModule(const std::string& from_file, const std::string& raw_path) { std::filesystem::path candidate = std::filesystem::path(raw_path).is_absolute() ? std::filesystem::path(raw_path) : std::filesystem::path(pathDirname(from_file)) / raw_path; candidate = candidate.lexically_normal(); std::error_code ec; if (candidate.extension() != ".drast") { auto with_ext = candidate; with_ext += ".drast"; if (std::filesystem::exists(with_ext, ec)) return with_ext.lexically_normal().string(); } if (std::filesystem::exists(candidate, ec)) return candidate.string(); return ""; }
inline std::vector<std::string> moduleDependencies(const std::string& source) { std::vector<std::string> deps; std::istringstream in(readFile(source)); std::string line; while (std::getline(in, line)) { bool header_hint = false; auto raw = parseUsePath(line, &header_hint); if (!raw || *raw == "std" || *raw == "drast" || *raw == "no_runtime" || header_hint || isHeaderPath(*raw)) continue; auto resolved = resolveDrastModule(source, *raw); if (!resolved.empty()) deps.push_back(normalizePath(resolved)); } std::sort(deps.begin(), deps.end()); deps.erase(std::unique(deps.begin(), deps.end()), deps.end()); return deps; }
inline std::vector<std::string> orderDrastSources(const std::string& entry, const std::string& root, bool auto_discover) { std::set<std::string> candidates; std::string normalized_entry = normalizePath(entry); candidates.insert(normalized_entry); if (auto_discover) for (const auto& source : discoverDrastSourceSiblings(root)) candidates.insert(normalizePath(source)); std::unordered_map<std::string, int> state; std::vector<std::string> ordered; std::function<void(const std::string&)> visit = [&](const std::string& source) { auto normalized = normalizePath(source); int seen = state[normalized]; if (seen == 2) return; if (seen == 1) return; if (!fileExists(normalized)) return; state[normalized] = 1; for (const auto& dep : moduleDependencies(normalized)) { candidates.insert(dep); visit(dep); } state[normalized] = 2; ordered.push_back(normalized); }; visit(normalized_entry); std::vector<std::string> sorted(candidates.begin(), candidates.end()); for (const auto& source : sorted) visit(source); ordered.erase(std::unique(ordered.begin(), ordered.end()), ordered.end()); return ordered; }
inline std::string findExecutable(const std::string& name) { if (name.empty()) return ""; if (name.find('/') != std::string::npos) return access(name.c_str(), X_OK) == 0 ? name : std::string(); std::string path = getEnv("PATH"); std::stringstream in(path); std::string dir; while (std::getline(in, dir, ':')) { if (dir.empty()) dir = "."; auto candidate = (std::filesystem::path(dir) / name).string(); if (access(candidate.c_str(), X_OK) == 0) return candidate; } return ""; }
inline std::string shell_quote(const std::string& text) { std::string out = "'"; for (char ch : text) { if (ch == '\'') out += "'\\''"; else out += ch; } out += "'"; return out; }
inline int runProcess(const std::string& program, const std::vector<std::string>& arguments, bool verbose = false) { std::string command = shell_quote(program); for (const auto& argument : arguments) { command += " "; command += shell_quote(argument); } if (verbose) std::cerr << command << '\n'; int status = std::system(command.c_str()); if (status == -1) return 1; if (WIFEXITED(status)) return WEXITSTATUS(status); return status == 0 ? 0 : 1; }
inline int runExecutable(const std::string& program, bool verbose = false) { std::vector<std::string> arguments; return runProcess(program, arguments, verbose); }
inline std::size_t line_count(const std::string& text) { if (text.empty()) return 0; std::size_t count = 1; for (char ch : text) if (ch == '\n') ++count; return count; }
inline std::string trim(const std::string& text) { std::size_t first = 0; while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) ++first; std::size_t last = text.size(); while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) --last; return text.substr(first, last - first); }
inline std::vector<std::string> split_whitespace(const std::string& text) { std::istringstream in(text); std::vector<std::string> words; std::string word; while (in >> word) words.push_back(word); return words; }
inline std::vector<std::string> split(const std::string& text, const std::string& delimiter) { std::vector<std::string> parts; if (delimiter.empty()) { for (char ch : text) parts.emplace_back(1, ch); return parts; } std::size_t start = 0; while (true) { std::size_t pos = text.find(delimiter, start); if (pos == std::string::npos) { parts.push_back(text.substr(start)); break; } parts.push_back(text.substr(start, pos - start)); start = pos + delimiter.size(); } return parts; }
inline std::string lowercase(std::string text) { std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); }); return text; }
template <typename T> std::string toString(const T& value) { std::ostringstream out; out << value; return out.str(); }
inline std::optional<int> parse_int(const std::string& text) { try { size_t used = 0; int value = std::stoi(text, &used); if (used != text.size()) return std::nullopt; return value; } catch (...) { return std::nullopt; } }
inline std::optional<int> parseInt(const std::string& text) { return parse_int(text); }
inline std::optional<double> parseFloat(const std::string& text) { try { size_t used = 0; double value = std::stod(text, &used); if (used != text.size()) return std::nullopt; return value; } catch (...) { return std::nullopt; } }
inline int charCode(char ch) { return static_cast<unsigned char>(ch); }
inline int charCode(const std::string& text) { return text.empty() ? 0 : charCode(text.front()); }
inline bool isAlpha(char ch) { return std::isalpha(static_cast<unsigned char>(ch)) != 0; }
inline bool isAlpha(const std::string& text) { return text.size() == 1 && isAlpha(text.front()); }
inline bool isDigit(char ch) { return std::isdigit(static_cast<unsigned char>(ch)) != 0; }
inline bool isDigit(const std::string& text) { return text.size() == 1 && isDigit(text.front()); }
inline bool isWhitespace(char ch) { return std::isspace(static_cast<unsigned char>(ch)) != 0; }
inline bool isWhitespace(const std::string& text) { return text.size() == 1 && isWhitespace(text.front()); }
inline bool contains(const std::string& text, const std::string& needle) { return text.find(needle) != std::string::npos; }
inline bool ends_with(const std::string& text, const std::string& suffix) { return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0; }
template <typename T> bool contains(const std::vector<T>& values, const T& needle) { return std::find(values.begin(), values.end(), needle) != values.end(); }
template <typename K, typename V> bool contains(const std::unordered_map<K, V>& values, const K& key) { return values.find(key) != values.end(); }
inline int find(const std::string& text, const std::string& needle) { auto pos = text.find(needle); return pos == std::string::npos ? -1 : static_cast<int>(pos); }
inline std::string replace_all(std::string text, const std::string& needle, const std::string& replacement) { if (needle.empty()) return text; std::size_t pos = 0; while ((pos = text.find(needle, pos)) != std::string::npos) { text.replace(pos, needle.size(), replacement); pos += replacement.size(); } return text; }
template <typename C> void remove_at(C& container, std::size_t index) { if (index >= container.size()) return; auto it = container.begin(); std::advance(it, static_cast<typename std::iterator_traits<decltype(it)>::difference_type>(index)); container.erase(it); }
template <typename C, typename T> void remove_value(C& container, const T& value) { container.erase(std::remove(container.begin(), container.end(), value), container.end()); }
template <typename K, typename V, typename F> V map_get(const std::unordered_map<K, V>& values, const K& key, const F& fallback) { auto found = values.find(key); if (found == values.end()) return V(fallback); return found->second; }
template <typename K, typename V> std::vector<K> map_keys(const std::unordered_map<K, V>& values) { std::vector<K> keys; keys.reserve(values.size()); for (const auto& entry : values) keys.push_back(entry.first); return keys; }
template <typename K, typename V> std::vector<V> map_values(const std::unordered_map<K, V>& values) { std::vector<V> values_out; values_out.reserve(values.size()); for (const auto& entry : values) values_out.push_back(entry.second); return values_out; }
inline float random_float(float lo, float hi) { thread_local std::mt19937 rng{std::random_device{}()}; std::uniform_real_distribution<float> dist(lo, hi); return dist(rng); }
inline int random_int(int lo, int hi) { thread_local std::mt19937 rng{std::random_device{}()}; std::uniform_int_distribution<int> dist(lo, hi); return dist(rng); }
struct CompileDiagnostic { std::string file; int line = 1; int column = 1; std::string message; };
inline std::vector<CompileDiagnostic>& diagnostic_store() { static std::vector<CompileDiagnostic> diagnostics; return diagnostics; }
inline void clearErrors() { diagnostic_store().clear(); }
inline void reportError(const std::string& file, int line, int column, const std::string& message) { diagnostic_store().push_back(CompileDiagnostic{file, line, column, message}); }
inline int errorCount() { return static_cast<int>(diagnostic_store().size()); }
inline bool hasErrors() { return !diagnostic_store().empty(); }
inline void emitErrors() { for (const CompileDiagnostic& diagnostic : diagnostic_store()) std::cerr << "[" << diagnostic.file << ":" << diagnostic.line << ":" << diagnostic.column << "] " << diagnostic.message << '\n'; }
} // namespace __drt

int countWords(const std::vector<std::string>& words);

int countWords(const std::vector<std::string>& words) {
    std::unordered_map<std::string, int> counts;
    for (const auto& word : words) {
        if (__drt::contains(counts, word)) {
            auto current = __drt::map_get(counts, word, 0);
            counts[word] = current + 1;
        } else {
            counts[word] = 1;
        }
    }
    int total = 0;
    for (const auto& [key, _] : counts) {
        total += __drt::map_get(counts, key, 0);
    }
    return total;
}

int main() {
    std::vector<std::string> words = {"one", "two", "one", "three"};
    int total = countWords(std::move(words));
    for (auto index = 0; index < 3; index += 1) {
        total += index;
    }
    return total;
}

