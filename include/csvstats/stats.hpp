#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "csvstats/csv.hpp"

namespace csvstats {

struct ColumnStats {
    std::string name;
    std::size_t count;
    long double sum;
    long double mean;
    long double min;
    long double max;
};

enum class StatsStatus {
    Ok,
    ColumnNotFound,
    ColumnNotNumeric,
    NoNumericColumn,
};

struct StatsResult {
    StatsStatus status;
    std::vector<ColumnStats> columns;
    std::string message;
};

StatsResult compute_stats(const Table& table,
                          const std::optional<std::string>& only_column);

}  // namespace csvstats
