#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace drast {

struct CompileDiagnostic {
    std::string file;
    int line = 1;
    int column = 1;
    std::string message;
};

inline std::string formatDiagnostic(const CompileDiagnostic &diagnostic) {
    std::ostringstream out;
    out << "[";
    if (!diagnostic.file.empty()) out << diagnostic.file << ":";
    out << diagnostic.line << ":" << diagnostic.column << "] "
        << diagnostic.message;
    return out.str();
}

inline std::vector<CompileDiagnostic> &diagnostic_store() {
    static std::vector<CompileDiagnostic> diagnostics;
    return diagnostics;
}

inline void clearErrors() {
    diagnostic_store().clear();
}

inline void reportError(const std::string &file, int line, int column,
                        const std::string &message) {
    diagnostic_store().push_back(CompileDiagnostic{file, line, column, message});
}

inline int errorCount() {
    return static_cast<int>(diagnostic_store().size());
}

inline bool hasErrors() {
    return !diagnostic_store().empty();
}

inline void emitErrors() {
    for (const CompileDiagnostic &diagnostic : diagnostic_store()) {
        std::cerr << formatDiagnostic(diagnostic) << '\n';
    }
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T>
void write_one(const T &value) {
    std::cout << value;
}

inline void write_one(const std::exception &value) {
    std::cout << value.what();
}

template <typename T>
void write_one(const std::optional<T> &value) {
    if (value) write_one(*value);
}

template <typename... Args>
void print(const Args &...args) {
    (write_one(args), ...);
}

template <typename... Args>
void println(const Args &...args) {
    print(args...);
    std::cout << '\n';
}

inline std::string getInput(const std::string &prompt = "") {
    if (!prompt.empty()) std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

inline float random_float(float lo, float hi) {
    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(rng);
}

inline int random_int(int lo, int hi) {
    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng);
}

inline std::optional<int> parse_int(const std::string &text) {
    try {
        size_t used = 0;
        int value = std::stoi(text, &used);
        if (used != text.size()) return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

inline std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

template <typename T>
std::string lowercase(const T &value) {
    std::ostringstream out;
    out << value;
    return lowercase(out.str());
}

inline bool starts_with(const std::string &text, const std::string &prefix) {
    return text.size() >= prefix.size() &&
           text.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(const std::string &text, const std::string &suffix) {
    return text.size() >= suffix.size() &&
           text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline bool contains(const std::string &text, const std::string &needle) {
    return text.find(needle) != std::string::npos;
}

template <typename T>
bool contains(const std::vector<T> &values, const T &needle) {
    return std::find(values.begin(), values.end(), needle) != values.end();
}

template <typename K, typename V>
bool contains(const std::unordered_map<K, V> &values, const K &key) {
    return values.find(key) != values.end();
}

inline int find(const std::string &text, const std::string &needle) {
    auto pos = text.find(needle);
    if (pos == std::string::npos) return -1;
    return static_cast<int>(pos);
}

inline std::string replace_all(std::string text, const std::string &needle,
                               const std::string &replacement) {
    if (needle.empty()) return text;
    std::size_t pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        text.replace(pos, needle.size(), replacement);
        pos += replacement.size();
    }
    return text;
}

inline std::string trim(const std::string &text) {
    std::size_t first = 0;
    while (first < text.size() &&
           std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }
    std::size_t last = text.size();
    while (last > first &&
           std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }
    return text.substr(first, last - first);
}

inline std::vector<std::string> split(const std::string &text,
                                      const std::string &delimiter) {
    std::vector<std::string> parts;
    if (delimiter.empty()) {
        for (char ch : text) parts.emplace_back(1, ch);
        return parts;
    }
    std::size_t start = 0;
    while (true) {
        std::size_t pos = text.find(delimiter, start);
        if (pos == std::string::npos) {
            parts.push_back(text.substr(start));
            break;
        }
        parts.push_back(text.substr(start, pos - start));
        start = pos + delimiter.size();
    }
    return parts;
}

inline std::vector<std::string> split_whitespace(const std::string &text) {
    std::istringstream in(text);
    std::vector<std::string> words;
    std::string word;
    while (in >> word) words.push_back(word);
    return words;
}

template <typename T>
std::string toString(const T &value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

inline std::optional<int> parseInt(const std::string &text) {
    return parse_int(text);
}

inline std::optional<double> parseFloat(const std::string &text) {
    try {
        size_t used = 0;
        double value = std::stod(text, &used);
        if (used != text.size()) return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

inline int charCode(char ch) {
    return static_cast<unsigned char>(ch);
}

inline int charCode(const std::string &text) {
    if (text.empty()) return 0;
    return charCode(text.front());
}

inline bool isAlpha(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0;
}

inline bool isAlpha(const std::string &text) {
    return text.size() == 1 && isAlpha(text.front());
}

inline bool isDigit(char ch) {
    return std::isdigit(static_cast<unsigned char>(ch)) != 0;
}

inline bool isDigit(const std::string &text) {
    return text.size() == 1 && isDigit(text.front());
}

inline bool isWhitespace(char ch) {
    return std::isspace(static_cast<unsigned char>(ch)) != 0;
}

inline bool isWhitespace(const std::string &text) {
    return text.size() == 1 && isWhitespace(text.front());
}

template <typename C>
void remove_at(C &container, std::size_t index) {
    if (index >= container.size()) return;
    auto it = container.begin();
    std::advance(it, static_cast<typename std::iterator_traits<decltype(it)>::difference_type>(index));
    container.erase(it);
}

template <typename C, typename T>
void remove_value(C &container, const T &value) {
    container.erase(std::remove(container.begin(), container.end(), value), container.end());
}

inline void playMP3(const std::string &) {}

inline std::vector<std::string> &program_args_store() {
    static std::vector<std::string> values;
    return values;
}

inline void setArgs(int argc, char **argv) {
    auto &values = program_args_store();
    values.clear();
    for (int i = 0; i < argc; ++i) values.emplace_back(argv[i] ? argv[i] : "");
}

inline const std::vector<std::string> &args() {
    return program_args_store();
}

inline std::string arg(std::size_t index) {
    const auto &values = program_args_store();
    if (index >= values.size()) return "";
    return values[index];
}

inline bool fileExists(const std::string &path) {
    std::ifstream in(path);
    return static_cast<bool>(in);
}

inline std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

inline bool writeFile(const std::string &path, const std::string &contents) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << contents;
    return static_cast<bool>(out);
}

inline std::string shell_quote(const std::string &text) {
    std::string out = "'";
    for (char ch : text) {
        if (ch == '\'') out += "'\\''";
        else out += ch;
    }
    out += "'";
    return out;
}

// Note: THIS SHOULD BE REMOVED.
// WE ARE SELF-HOSTING, this is never going to be needed now.
inline int delegateToV1(int = 0) {
    std::string command = shell_quote("./build/drastc");
    const auto &values = args();
    for (std::size_t i = 1; i < values.size(); ++i) {
        command += " ";
        command += shell_quote(values[i]);
    }
    int status = std::system(command.c_str());
    return status == 0 ? 0 : 1;
}

inline std::size_t line_count(const std::string &text) {
    if (text.empty()) return 0;
    std::size_t count = 1;
    for (char ch : text) {
        if (ch == '\n') ++count;
    }
    return count;
}

template <typename K, typename V, typename F>
V map_get(const std::unordered_map<K, V> &values, const K &key,
          const F &fallback) {
    auto found = values.find(key);
    if (found == values.end()) return V(fallback);
    return found->second;
}

template <typename K, typename V>
std::vector<K> map_keys(const std::unordered_map<K, V> &values) {
    std::vector<K> keys;
    keys.reserve(values.size());
    for (const auto &entry : values) keys.push_back(entry.first);
    return keys;
}

template <typename K, typename V>
std::vector<V> map_values(const std::unordered_map<K, V> &values) {
    std::vector<V> out;
    out.reserve(values.size());
    for (const auto &entry : values) out.push_back(entry.second);
    return out;
}

template <typename T>
std::optional<T> make_optional_cast(const T &value) {
    return value;
}

} // namespace drast

inline std::string isn(const std::string &suffix) {
    return "isn" + suffix;
}
