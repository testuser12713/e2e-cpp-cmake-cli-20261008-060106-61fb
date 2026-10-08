#pragma once

#include <optional>
#include <string>

namespace csvstats::cli {

struct Options {
    std::optional<std::string> path;
    char delimiter = ',';
    std::optional<std::string> column;
    bool help = false;
};

enum class ArgStatus {
    Ok,
    Help,
    BadUsage,
    MissingOptionValue,
};

struct ArgsResult {
    ArgStatus status;
    Options options;
    std::string message;
};

ArgsResult parse_args(int argc, const char* const argv[]);

std::string usage_text();

int run(int argc, const char* const argv[]);

}  // namespace csvstats::cli
