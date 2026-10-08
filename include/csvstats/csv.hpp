#pragma once

#include <string>
#include <vector>

namespace csvstats {

struct Table {
    std::vector<std::string> header;
    std::vector<std::vector<std::string>> rows;
};

enum class ParseStatus {
    Ok,
    EmptyFile,
    FileMissing,
    FileUnreadable,
};

struct ParseResult {
    ParseStatus status;
    Table table;
    std::string message;
};

ParseResult parse_file(const std::string& path, char delimiter);

std::vector<std::string> parse_line(const std::string& line, char delimiter);

}  // namespace csvstats
