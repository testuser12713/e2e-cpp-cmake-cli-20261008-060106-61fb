// Skeleton tests for the CLI layer.
//
// usage_text(), the default options and the run() exit-code mapping are part
// of the completed skeleton, so these assertions stay valid while ticket #2
// fills in parse_args.

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

}  // namespace

int main() {
    check(csvstats::cli::usage_text() == "Usage: csvstats [OPTIONS] <file>",
          "usage_text returns the contract usage line");

    const char* const no_args[] = {"csvstats"};
    const csvstats::cli::ArgsResult parsed = csvstats::cli::parse_args(1, no_args);
    check(parsed.status == csvstats::cli::ArgStatus::Ok,
          "parse_args succeeds when only the program name is given");
    check(!parsed.options.path.has_value(),
          "no input path is set when no positional argument is given");
    check(parsed.options.delimiter == ',',
          "the default delimiter is a comma");

    const int code = csvstats::cli::run(1, no_args);
    check(code == 2, "run with no input path exits with code 2");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_cli: all checks passed\n";
    return 0;
}
