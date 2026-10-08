// Skeleton tests for the csvstats_core library.
//
// They assert that the parser entry points are linked and callable without the
// CLI (AC-10) and that they keep the contract's invariants. They deliberately
// do NOT pin the temporary value a later ticket's implementation replaces:
// a test of a stub's answer is a test that must fail on main once that ticket
// merges.

#include <csvstats/csv.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cerr << "FAIL: " << what << '\n';
        ++failures;
    }
}

bool is_declared(csvstats::ParseStatus status) {
    switch (status) {
        case csvstats::ParseStatus::Ok:
        case csvstats::ParseStatus::EmptyFile:
        case csvstats::ParseStatus::FileMissing:
        case csvstats::ParseStatus::FileUnreadable:
            return true;
    }
    return false;
}

}  // namespace

int main() {
    const csvstats::ParseResult file = csvstats::parse_file("sample.csv", ',');
    check(is_declared(file.status),
          "parse_file returns a declared ParseStatus");

    const std::vector<std::string> fields = csvstats::parse_line("a,b,c", ',');
    check(fields.size() <= 3,
          "parse_line never yields more fields than separators + 1");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_csv: all checks passed\n";
    return 0;
}
