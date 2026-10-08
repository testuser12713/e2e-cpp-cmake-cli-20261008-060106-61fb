#include "csvstats/cli.hpp"

#include <iostream>

#include "csvstats/csv.hpp"
#include "csvstats/report.hpp"
#include "csvstats/stats.hpp"

namespace csvstats::cli {

// Skeleton stub. Ticket #2 replaces this with the real option parser
// (--delimiter, --column, --help, positional <file>).
ArgsResult parse_args(int /*argc*/, const char* const /*argv*/[]) {
    ArgsResult result;
    result.status = ArgStatus::Ok;
    return result;
}

std::string usage_text() {
    return "Usage: csvstats [OPTIONS] <file>";
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
