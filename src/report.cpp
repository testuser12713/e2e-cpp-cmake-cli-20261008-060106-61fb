#include "csvstats/report.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace csvstats {
namespace {

// Fixed number of decimals for sum, mean, min and max (AC-09).
constexpr int kDecimals = 6;
constexpr std::size_t kFieldCount = 6;

// Locale-independent fixed-point formatting so identical input yields a
// byte-identical report on every machine.
std::string format_fixed(long double value) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(kDecimals) << value;
    return stream.str();
}

std::string pad_left(const std::string& value, std::size_t width) {
    if (value.size() >= width) {
        return value;
    }
    return std::string(width - value.size(), ' ') + value;
}

}  // namespace

std::string format_report(const std::vector<ColumnStats>& columns) {
    if (columns.empty()) {
        return std::string{};
    }

    const std::array<std::string, kFieldCount> labels{"name", "count", "sum",
                                                      "mean", "min",   "max"};

    std::vector<std::array<std::string, kFieldCount>> rows;
    rows.reserve(columns.size() + 1);
    rows.push_back(labels);
    for (const ColumnStats& column : columns) {
        std::array<std::string, kFieldCount> row{
            column.name,
            std::to_string(column.count),
            format_fixed(column.sum),
            format_fixed(column.mean),
            format_fixed(column.min),
            format_fixed(column.max),
        };
        rows.push_back(row);
    }

    std::array<std::size_t, kFieldCount> widths{};
    for (const auto& row : rows) {
        for (std::size_t i = 0; i < kFieldCount; ++i) {
            widths[i] = std::max(widths[i], row[i].size());
        }
    }

    std::string out;
    for (const auto& row : rows) {
        for (std::size_t i = 0; i < kFieldCount; ++i) {
            if (i != 0) {
                out += ' ';
            }
            out += pad_left(row[i], widths[i]);
        }
        out += '\n';
    }
    return out;
}

}  // namespace csvstats
