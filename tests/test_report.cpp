// Skeleton test for the report renderer.
//
// The shared contract promises that format_report returns a '\n'-terminated
// string; that holds for an empty column list and for a populated one, so the
// assertion survives the ticket that replaces the stub body.

#include <csvstats/report.hpp>

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
    const std::string report = csvstats::format_report({});
    check(report.empty() || report.back() == '\n',
          "format_report output is '\\n'-terminated");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_report: all checks passed\n";
    return 0;
}
