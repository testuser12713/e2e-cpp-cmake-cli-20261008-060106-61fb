#include "csvstats/stats.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace csvstats {
namespace {

// Strips leading and trailing ASCII whitespace. The CSV reader (AC-08) is
// responsible for surrounding whitespace; trimming here too keeps the numeric
// grammar stable when a Table is built directly.
std::string trim(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(begin, end - begin);
}

// Strict numeric grammar: optional sign, digits with an optional decimal
// point, an optional exponent. No decimal comma, no thousands separator and
// the whole (trimmed) field must be consumed.
bool is_number(const std::string& text, long double& out) {
    const std::size_t n = text.size();
    std::size_t i = 0;
    if (i < n && (text[i] == '+' || text[i] == '-')) {
        ++i;
    }

    bool integer_digits = false;
    while (i < n && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
        integer_digits = true;
    }

    if (i < n && text[i] == '.') {
        ++i;
        bool fractional_digits = false;
        while (i < n && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
            ++i;
            fractional_digits = true;
        }
        if (!integer_digits && !fractional_digits) {
            return false;
        }
    } else if (!integer_digits) {
        return false;
    }

    if (i < n && (text[i] == 'e' || text[i] == 'E')) {
        ++i;
        if (i < n && (text[i] == '+' || text[i] == '-')) {
            ++i;
        }
        bool exponent_digits = false;
        while (i < n && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
            ++i;
            exponent_digits = true;
        }
        if (!exponent_digits) {
            return false;
        }
    }

    if (i != n) {
        return false;
    }

    char* end = nullptr;
    const long double parsed = std::strtold(text.c_str(), &end);
    if (end != text.c_str() + text.size()) {
        return false;
    }
    out = parsed;
    return true;
}

enum class FieldKind { Blank, Number, Invalid };

FieldKind classify(const std::string& raw, long double& out) {
    const std::string text = trim(raw);
    if (text.empty()) {
        return FieldKind::Blank;
    }
    if (is_number(text, out)) {
        return FieldKind::Number;
    }
    return FieldKind::Invalid;
}

// A column is numeric when it has at least one non-empty value and every
// non-empty value parses as a number. Returns false for text, mixed and
// all-empty columns.
bool evaluate_column(const Table& table, std::size_t index, ColumnStats& out) {
    bool numeric = true;
    std::size_t count = 0;
    long double sum = 0.0L;
    long double min = 0.0L;
    long double max = 0.0L;

    for (const std::vector<std::string>& row : table.rows) {
        const std::string cell = index < row.size() ? row[index] : std::string{};
        long double value = 0.0L;
        const FieldKind kind = classify(cell, value);
        if (kind == FieldKind::Invalid) {
            numeric = false;
            break;
        }
        if (kind == FieldKind::Number) {
            if (count == 0) {
                min = value;
                max = value;
            } else {
                min = std::min(min, value);
                max = std::max(max, value);
            }
            sum += value;
            ++count;
        }
    }

    if (!numeric || count == 0) {
        return false;
    }

    out.name = table.header[index];
    out.count = count;
    out.sum = sum;
    out.mean = sum / static_cast<long double>(count);
    out.min = min;
    out.max = max;
    return true;
}

}  // namespace

StatsResult compute_stats(const Table& table, const std::optional<std::string>& only_column) {
    StatsResult result;
    result.status = StatsStatus::Ok;

    if (only_column.has_value()) {
        const std::string& wanted = *only_column;
        std::optional<std::size_t> index;
        for (std::size_t i = 0; i < table.header.size(); ++i) {
            if (table.header[i] == wanted) {
                index = i;
                break;
            }
        }
        if (!index.has_value()) {
            result.status = StatsStatus::ColumnNotFound;
            result.message = "column '" + wanted + "' not found";
            return result;
        }

        ColumnStats stats;
        if (!evaluate_column(table, *index, stats)) {
            result.status = StatsStatus::ColumnNotNumeric;
            result.message = "column '" + wanted + "' is not numeric";
            return result;
        }
        result.columns.push_back(stats);
        return result;
    }

    for (std::size_t i = 0; i < table.header.size(); ++i) {
        ColumnStats stats;
        if (evaluate_column(table, i, stats)) {
            result.columns.push_back(stats);
        }
    }

    if (result.columns.empty()) {
        result.status = StatsStatus::NoNumericColumn;
        result.message = "no numeric column found";
    }
    return result;
}

}  // namespace csvstats
