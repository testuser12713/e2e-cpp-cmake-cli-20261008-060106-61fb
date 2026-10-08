// Unit tests for the CLI argument parser and usage text (ticket #2).
//
// Covers the default delimiter, --delimiter with ';' and the \t sequence,
// --column capture, --help/-h, unknown options, missing option values and a
// missing input path. run()'s exit-code mapping is exercised only where it
// does not depend on the CSV/stats tickets still in flight.

#include <csvstats/cli.hpp>

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cerr << "FAIL: " << what << '\n';
        ++failures;
    }
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

using csvstats::cli::ArgStatus;
using csvstats::cli::ArgsResult;
using csvstats::cli::parse_args;

ArgStatus status_of(const char* const argv[], int argc) {
    return parse_args(argc, argv).status;
}

}  // namespace

int main() {
    // --- usage_text ---------------------------------------------------------
    const std::string usage = csvstats::cli::usage_text();
    check(contains(usage, "Usage: csvstats [OPTIONS] <file>"),
          "usage_text contains the usage line");
    check(contains(usage, "--delimiter"),
          "usage_text lists the --delimiter option");
    check(contains(usage, "--column"),
          "usage_text lists the --column option");
    check(contains(usage, "--help"),
          "usage_text lists the --help option");
    check(contains(usage, "Exit codes"),
          "usage_text contains the exit-code table");
    check(contains(usage, "0") && contains(usage, "1") && contains(usage, "2") &&
              contains(usage, "3") && contains(usage, "4"),
          "usage_text documents every exit code");

    // --- default delimiter and missing path --------------------------------
    {
        const char* const argv[] = {"csvstats"};
        const ArgsResult parsed = parse_args(1, argv);
        check(parsed.status == ArgStatus::Ok,
              "parse_args succeeds when only the program name is given");
        check(!parsed.options.path.has_value(),
              "no input path is set when no positional argument is given");
        check(parsed.options.delimiter == ',',
              "the default delimiter is a comma");

        const int code = csvstats::cli::run(1, argv);
        check(code == 2, "run with no input path exits with code 2");
    }

    // --- positional path ----------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "data.csv"};
        const ArgsResult parsed = parse_args(2, argv);
        check(parsed.status == ArgStatus::Ok, "a bare path argument is accepted");
        check(parsed.options.path.has_value() &&
                  *parsed.options.path == "data.csv",
              "the first non-option argument is captured as the input path");
        check(parsed.options.delimiter == ',',
              "the delimiter stays at the default when not given");
    }

    // --- --delimiter ';' ----------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "--delimiter", ";", "data.csv"};
        const ArgsResult parsed = parse_args(4, argv);
        check(parsed.status == ArgStatus::Ok,
              "--delimiter ';' parses successfully");
        check(parsed.options.delimiter == ';',
              "--delimiter captures the semicolon");
        check(parsed.options.path.has_value() &&
                  *parsed.options.path == "data.csv",
              "the path may follow the delimiter option");
    }

    // --- --delimiter \t -----------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "--delimiter", "\\t", "data.tsv"};
        const ArgsResult parsed = parse_args(4, argv);
        check(parsed.status == ArgStatus::Ok,
              "--delimiter \\t parses successfully");
        check(parsed.options.delimiter == '\t',
              "--delimiter \\t is accepted as the tab character");
    }

    // --- --column -----------------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "--column", "price", "data.csv"};
        const ArgsResult parsed = parse_args(4, argv);
        check(parsed.status == ArgStatus::Ok,
              "--column parses successfully");
        check(parsed.options.column.has_value() &&
                  *parsed.options.column == "price",
              "--column captures its value");
    }

    // --- options after the path --------------------------------------------
    {
        const char* const argv[] = {"csvstats", "data.csv", "--column", "qty"};
        const ArgsResult parsed = parse_args(4, argv);
        check(parsed.status == ArgStatus::Ok,
              "options may follow the positional path");
        check(parsed.options.path.has_value() &&
                  *parsed.options.path == "data.csv",
              "the path is still captured before a later option");
        check(parsed.options.column.has_value() &&
                  *parsed.options.column == "qty",
              "an option after the path still captures its value");
    }

    // --- --help and -h ------------------------------------------------------
    {
        const char* const help_long[] = {"csvstats", "--help"};
        check(status_of(help_long, 2) == ArgStatus::Help,
              "--help yields ArgStatus::Help");

        const char* const help_short[] = {"csvstats", "-h"};
        check(status_of(help_short, 2) == ArgStatus::Help,
              "-h yields ArgStatus::Help");

        check(csvstats::cli::run(2, help_long) == 0,
              "run with --help exits with code 0");
    }

    // --- unknown option -----------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "--bogus"};
        const ArgsResult parsed = parse_args(2, argv);
        check(parsed.status == ArgStatus::BadUsage,
              "an unknown option yields ArgStatus::BadUsage");
        check(!parsed.message.empty(),
              "an unknown option carries an English message");
    }

    // --- missing option values ---------------------------------------------
    {
        const char* const delimiter_last[] = {"csvstats", "--delimiter"};
        const ArgsResult parsed = parse_args(2, delimiter_last);
        check(parsed.status == ArgStatus::MissingOptionValue,
              "a trailing --delimiter yields ArgStatus::MissingOptionValue");
        check(!parsed.message.empty(),
              "a missing --delimiter value carries an English message");

        const char* const column_last[] = {"csvstats", "data.csv", "--column"};
        check(status_of(column_last, 3) == ArgStatus::MissingOptionValue,
              "a trailing --column yields ArgStatus::MissingOptionValue");
    }

    // --- invalid delimiter --------------------------------------------------
    {
        const char* const argv[] = {"csvstats", "--delimiter", "ab", "data.csv"};
        check(status_of(argv, 4) == ArgStatus::BadUsage,
              "an over-long delimiter is rejected as bad usage");
    }

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_cli: all checks passed\n";
    return 0;
}
