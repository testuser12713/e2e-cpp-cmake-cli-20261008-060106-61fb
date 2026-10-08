#include "csvstats/cli.hpp"

#include <iostream>
#include <string>

#include "csvstats/csv.hpp"
#include "csvstats/report.hpp"
#include "csvstats/stats.hpp"

namespace csvstats::cli {
namespace {

ArgsResult bad_usage(const std::string& message) {
    ArgsResult result;
    result.status = ArgStatus::BadUsage;
    result.message = message;
    return result;
}

ArgsResult missing_value(const std::string& option) {
    ArgsResult result;
    result.status = ArgStatus::MissingOptionValue;
    result.message = "csvstats: option '" + option + "' requires a value";
    return result;
}

}  // namespace

// Option parser for: csvstats [--delimiter X] [--column NAME] [--help|-h] <file>
//
// The first non-option argument is the input path. `--delimiter` accepts a
// single character, or the two-character sequence \t for a tab. An unknown
// option yields BadUsage, a value-taking option at the end of the argument
// list yields MissingOptionValue - each with a short English message.
ArgsResult parse_args(int argc, const char* const argv[]) {
    ArgsResult result;
    result.status = ArgStatus::Ok;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            result.status = ArgStatus::Help;
            result.options.help = true;
            return result;
        }
        if (arg == "--delimiter") {
            if (i + 1 >= argc) {
                return missing_value("--delimiter");
            }
            const std::string value = argv[++i];
            if (value == "\\t") {
                result.options.delimiter = '\t';
            } else if (value.size() == 1) {
                result.options.delimiter = value[0];
            } else {
                return bad_usage("csvstats: invalid delimiter '" + value +
                                 "' (expected a single character or \\t)");
            }
            continue;
        }
        if (arg == "--column") {
            if (i + 1 >= argc) {
                return missing_value("--column");
            }
            result.options.column = std::string(argv[++i]);
            continue;
        }
        if (!arg.empty() && arg[0] == '-') {
            return bad_usage("csvstats: unknown option '" + arg + "'");
        }
        if (result.options.path.has_value()) {
            return bad_usage("csvstats: unexpected argument '" + arg + "'");
        }
        result.options.path = arg;
    }

    return result;
}

std::string usage_text() {
    return std::string{
        "Usage: csvstats [OPTIONS] <file>\n"
        "\n"
        "Options:\n"
        "  --delimiter X   field delimiter, a single character; \\t means tab "
        "(default: ,)\n"
        "  --column NAME   report only the named column\n"
        "  -h, --help      print this help and exit\n"
        "\n"
        "Exit codes:\n"
        "  0  success, or no numeric column found\n"
        "  1  usage error: unknown option or missing option value\n"
        "  2  missing or unreadable file\n"
        "  3  empty file or file without data rows\n"
        "  4  unknown column or non-numeric column"};
}

// The complete argument dispatch: it is written once by the skeleton and only
// ever consumes parse_args(), parse_file(), compute_stats() and format_report().
int run(int argc, const char* const argv[]) {
    const ArgsResult args = parse_args(argc, argv);

    switch (args.status) {
        case ArgStatus::Help:
            std::cout << usage_text() << '\n';
            return 0;
        case ArgStatus::BadUsage:
        case ArgStatus::MissingOptionValue:
            std::cerr << args.message << '\n' << usage_text() << '\n';
            return 1;
        case ArgStatus::Ok:
            break;
    }

    const Options& options = args.options;
    if (!options.path.has_value()) {
        std::cerr << "csvstats: no input file given\n" << usage_text() << '\n';
        return 2;
    }

    const ParseResult parsed = parse_file(*options.path, options.delimiter);
    switch (parsed.status) {
        case ParseStatus::FileMissing:
        case ParseStatus::FileUnreadable:
            std::cerr << parsed.message << '\n';
            return 2;
        case ParseStatus::EmptyFile:
            std::cerr << parsed.message << '\n';
            return 3;
        case ParseStatus::Ok:
            break;
    }

    const StatsResult stats = compute_stats(parsed.table, options.column);
    switch (stats.status) {
        case StatsStatus::ColumnNotFound:
        case StatsStatus::ColumnNotNumeric:
            std::cerr << stats.message << '\n';
            return 4;
        case StatsStatus::NoNumericColumn:
            std::cerr << stats.message << '\n';
            return 0;
        case StatsStatus::Ok:
            break;
    }

    std::cout << format_report(stats.columns);
    return 0;
}

}  // namespace csvstats::cli
