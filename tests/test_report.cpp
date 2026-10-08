// Unit tests for the report renderer (AC-01, AC-09). No third-party test
// framework is used.

#include <csvstats/report.hpp>

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

std::size_t count_char(const std::string& text, char needle) {
    std::size_t total = 0;
    for (char c : text) {
        if (c == needle) {
            ++total;
        }
    }
    return total;
}

}  // namespace

int main() {
    check(csvstats::format_report({}).empty(),
          "format_report of no columns is empty");

    const std::vector<csvstats::ColumnStats> columns{
        {"a", 2, 3.0L, 1.5L, 1.0L, 2.0L},
        {"long", 1, -10.25L, -10.25L, -10.25L, -10.25L},
    };

    const std::string report = csvstats::format_report(columns);

    const std::string expected =
        "name count        sum       mean        min        max\n"
        "   a     2   3.000000   1.500000   1.000000   2.000000\n"
        "long     1 -10.250000 -10.250000 -10.250000 -10.250000\n";
    check(report == expected, "report matches the exact aligned layout");

    check(!report.empty() && report.back() == '\n',
          "report output is '\\n'-terminated");
    check(count_char(report, '\n') == 3,
          "report has one header line plus one line per column");

    check(csvstats::format_report(columns) == report,
          "report is byte-identical for identical input");

    // Right alignment: within a column every line ends at the same column. The
    // widest name is 'long' (4), so header 'name' and cell 'a' are padded.
    const std::string single = csvstats::format_report({{"n", 1, 1.25L, 1.25L, 1.25L, 1.25L}});
    const std::string single_expected =
        "name count      sum     mean      min      max\n"
        "   n     1 1.250000 1.250000 1.250000 1.250000\n";
    check(single == single_expected, "single column is aligned to its own width");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_report: all checks passed\n";
    return 0;
}
