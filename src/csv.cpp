#include "csvstats/csv.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace csvstats {

namespace {

bool is_blank(const std::string& line, char delimiter) {
    for (const char c : line) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (c == delimiter || !std::isspace(uc)) {
            return false;
        }
    }
    return true;
}

bool is_skippable_space(char c, char delimiter) {
    return c != delimiter && std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::vector<std::string> split_lines(const std::string& content) {
    std::vector<std::string> lines;
    std::string current;
    for (const char c : content) {
        if (c == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

ParseResult make_error(ParseStatus status, std::string message) {
    ParseResult result;
    result.status = status;
    result.message = std::move(message);
    return result;
}

}  // namespace

ParseResult parse_file(const std::string& path, char delimiter) {
    std::error_code ec;
    const bool exists = std::filesystem::exists(path, ec);

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        if (!exists || ec) {
            return make_error(ParseStatus::FileMissing,
                              "csvstats: cannot find file '" + path + "'.");
        }
        return make_error(ParseStatus::FileUnreadable,
                          "csvstats: cannot read file '" + path + "'.");
    }

    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    if (in.bad()) {
        return make_error(ParseStatus::FileUnreadable,
                          "csvstats: failed while reading file '" + path + "'.");
    }

    if (content.empty()) {
        return make_error(ParseStatus::EmptyFile,
                          "csvstats: file is empty: '" + path + "'.");
    }

    if (content.size() >= 3 && static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content.erase(0, 3);
    }

    std::vector<std::vector<std::string>> records;
    for (const std::string& line : split_lines(content)) {
        std::vector<std::string> fields = parse_line(line, delimiter);
        if (!fields.empty()) {
            records.push_back(std::move(fields));
        }
    }

    if (records.empty()) {
        return make_error(ParseStatus::EmptyFile,
                          "csvstats: file is empty: '" + path + "'.");
    }
    if (records.size() < 2) {
        return make_error(ParseStatus::EmptyFile,
                          "csvstats: file has a header but no data rows: '" + path + "'.");
    }

    ParseResult result;
    result.status = ParseStatus::Ok;
    result.table.header = std::move(records.front());
    result.table.rows.assign(std::make_move_iterator(records.begin() + 1),
                             std::make_move_iterator(records.end()));
    return result;
}

std::vector<std::string> parse_line(const std::string& line, char delimiter) {
    std::string s = line;
    if (!s.empty() && s.back() == '\r') {
        s.pop_back();
    }
    if (is_blank(s, delimiter)) {
        return {};
    }

    const std::size_t n = s.size();
    std::vector<std::string> fields;
    std::size_t i = 0;
    while (true) {
        while (i < n && is_skippable_space(s[i], delimiter)) {
            ++i;
        }

        std::string field;
        if (i < n && s[i] == '"') {
            ++i;  // opening quote
            while (i < n) {
                const char c = s[i];
                if (c == '"') {
                    if (i + 1 < n && s[i + 1] == '"') {
                        field.push_back('"');
                        i += 2;
                        continue;
                    }
                    ++i;  // closing quote
                    break;
                }
                field.push_back(c);
                ++i;
            }
            while (i < n && is_skippable_space(s[i], delimiter)) {
                ++i;
            }
        } else {
            const std::size_t start = i;
            while (i < n && s[i] != delimiter) {
                ++i;
            }
            field = s.substr(start, i - start);
            std::size_t end = field.size();
            while (end > 0 && is_skippable_space(field[end - 1], delimiter)) {
                --end;
            }
            field.resize(end);
        }

        fields.push_back(std::move(field));
        if (i < n && s[i] == delimiter) {
            ++i;
            continue;
        }
        break;
    }
    return fields;
}

}  // namespace csvstats
