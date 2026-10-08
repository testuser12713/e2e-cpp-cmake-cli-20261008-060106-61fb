// Skeleton tests for the csvstats_core statistics entry point.
//
// The empty-table invariant holds for the stub and for the real
// implementation, so this test stays green across the ticket that fills it in
// (AC-10). It does not pin the stub's temporary status.

#include <csvstats/stats.hpp>

#include <iostream>
#include <optional>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cerr << "FAIL: " << what << '\n';
        ++failures;
    }
}

bool is_declared(csvstats::StatsStatus status) {
    switch (status) {
        case csvstats::StatsStatus::Ok:
        case csvstats::StatsStatus::ColumnNotFound:
        case csvstats::StatsStatus::ColumnNotNumeric:
        case csvstats::StatsStatus::NoNumericColumn:
            return true;
    }
    return false;
}

}  // namespace

int main() {
    const csvstats::Table table;
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);

    check(is_declared(result.status),
          "compute_stats returns a declared StatsStatus");
    check(result.columns.empty(),
          "compute_stats has no columns to report for an empty table");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_stats: all checks passed\n";
    return 0;
}
