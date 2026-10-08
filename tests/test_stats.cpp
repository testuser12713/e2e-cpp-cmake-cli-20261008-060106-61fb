// Unit tests for the csvstats_core statistics entry point (AC-01, AC-02,
// AC-04, AC-09, AC-12). No third-party test framework is used.

#include <csvstats/stats.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
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

bool nearly(long double actual, long double expected) {
    const long double scale = std::max<long double>(1.0L, std::fabs(expected));
    return std::fabs(actual - expected) <= 1e-9L * scale;
}

void check_stats(const csvstats::ColumnStats& stats, const std::string& name,
                 std::size_t count, long double sum, long double mean,
                 long double min, long double max, const std::string& what) {
    check(stats.name == name, what + ": name");
    check(stats.count == count, what + ": count");
    check(nearly(stats.sum, sum), what + ": sum");
    check(nearly(stats.mean, mean), what + ": mean");
    check(nearly(stats.min, min), what + ": min");
    check(nearly(stats.max, max), what + ": max");
}

csvstats::Table make_table(std::vector<std::string> header,
                           std::vector<std::vector<std::string>> rows) {
    csvstats::Table table;
    table.header = std::move(header);
    table.rows = std::move(rows);
    return table;
}

void test_empty_table() {
    const csvstats::Table table;
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::NoNumericColumn,
          "empty table yields NoNumericColumn (AC-12)");
    check(result.columns.empty(), "empty table has no columns (AC-10)");
    check(!result.message.empty(), "empty table carries a hint message");
}

void test_single_row_single_column() {
    const csvstats::Table table = make_table({"value"}, {{"5"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::Ok, "single value: Ok");
    check(result.columns.size() == 1, "single value: one column");
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "value", 1, 5.0L, 5.0L, 5.0L, 5.0L, "single value");
    }
}

void test_negative_and_fractional() {
    const csvstats::Table table = make_table({"x"}, {{"-2.5"}, {"3"}, {"0.25"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::Ok, "negative/fractional: Ok");
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "x", 3, 0.75L, 0.25L, -2.5L, 3.0L,
                    "negative/fractional");
    }
}

void test_ignored_empty_fields() {
    const csvstats::Table table = make_table({"v"}, {{"1"}, {""}, {"   "}, {"3"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "v", 2, 4.0L, 2.0L, 1.0L, 3.0L,
                    "ignored empty fields");
    } else {
        check(false, "ignored empty fields: one numeric column expected");
    }
}

void test_constant_column() {
    const csvstats::Table table = make_table({"c"}, {{"7"}, {"7"}, {"7"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "c", 3, 21.0L, 7.0L, 7.0L, 7.0L,
                    "constant column");
    } else {
        check(false, "constant column: one numeric column expected");
    }
}

void test_large_value_range() {
    const csvstats::Table table = make_table({"big"}, {{"-1e9"}, {"2e9"}, {"3.5"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "big", 3, 1.0e9L + 3.5L,
                    (1.0e9L + 3.5L) / 3.0L, -1.0e9L, 2.0e9L, "large value range");
    } else {
        check(false, "large value range: one numeric column expected");
    }
}

void test_text_and_mixed_skipped() {
    const csvstats::Table table = make_table(
        {"num", "text", "mixed"},
        {{"1", "a", "1"}, {"2", "b", "x"}, {"3", "c", "3"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::Ok, "text/mixed: Ok");
    check(result.columns.size() == 1, "text/mixed: only the numeric column survives");
    if (result.columns.size() == 1) {
        check(result.columns[0].name == "num", "text/mixed: surviving column is 'num'");
    }
}

void test_column_selection() {
    const csvstats::Table table = make_table({"a", "b"}, {{"1", "2"}, {"3", "4"}});
    const csvstats::StatsResult result =
        csvstats::compute_stats(table, std::optional<std::string>{"b"});
    check(result.status == csvstats::StatsStatus::Ok, "column selection: Ok");
    check(result.columns.size() == 1, "column selection: exactly one column");
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "b", 2, 6.0L, 3.0L, 2.0L, 4.0L,
                    "column selection");
    }
}

void test_unknown_column() {
    const csvstats::Table table = make_table({"a"}, {{"1"}});
    const csvstats::StatsResult result =
        csvstats::compute_stats(table, std::optional<std::string>{"missing"});
    check(result.status == csvstats::StatsStatus::ColumnNotFound,
          "unknown column: ColumnNotFound (AC-04)");
    check(result.message.find("missing") != std::string::npos,
          "unknown column: message names the column");
    check(result.columns.empty(), "unknown column: no output columns");
}

void test_non_numeric_column() {
    const csvstats::Table table = make_table({"label"}, {{"x"}, {"y"}});
    const csvstats::StatsResult result =
        csvstats::compute_stats(table, std::optional<std::string>{"label"});
    check(result.status == csvstats::StatsStatus::ColumnNotNumeric,
          "non-numeric column: ColumnNotNumeric (AC-04)");
    check(result.message.find("label") != std::string::npos,
          "non-numeric column: message names the column");
}

void test_all_empty_column_selected() {
    const csvstats::Table table = make_table({"e"}, {{""}, {""}});
    const csvstats::StatsResult result =
        csvstats::compute_stats(table, std::optional<std::string>{"e"});
    check(result.status == csvstats::StatsStatus::ColumnNotNumeric,
          "all-empty column: not numeric (AC-02)");
}

void test_no_numeric_column() {
    const csvstats::Table table = make_table({"a", "b"}, {{"x", "y"}, {"z", "w"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::NoNumericColumn,
          "all text: NoNumericColumn (AC-12)");
    check(!result.message.empty(), "all text: hint message present");
}

void test_invalid_numeric_shapes() {
    const csvstats::Table table = make_table(
        {"decimal_comma", "thousands", "garbage"},
        {{"1,5", "1,000", "3abc"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    check(result.status == csvstats::StatsStatus::NoNumericColumn,
          "decimal comma, thousands separator and trailing garbage are rejected");
}

void test_exponent_forms() {
    const csvstats::Table table = make_table({"e"}, {{"+1.5e2"}, {"-2E-1"}, {".5"}});
    const csvstats::StatsResult result = csvstats::compute_stats(table, std::nullopt);
    if (result.columns.size() == 1) {
        check_stats(result.columns[0], "e", 3, 150.3L, 50.1L, -0.2L, 150.0L,
                    "exponent and sign forms");
    } else {
        check(false, "exponent forms: one numeric column expected");
    }
}

}  // namespace

int main() {
    test_empty_table();
    test_single_row_single_column();
    test_negative_and_fractional();
    test_ignored_empty_fields();
    test_constant_column();
    test_large_value_range();
    test_text_and_mixed_skipped();
    test_column_selection();
    test_unknown_column();
    test_non_numeric_column();
    test_all_empty_column_selected();
    test_no_numeric_column();
    test_invalid_numeric_shapes();
    test_exponent_forms();

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_stats: all checks passed\n";
    return 0;
}
