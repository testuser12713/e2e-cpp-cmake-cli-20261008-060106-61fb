#pragma once

#include <string>
#include <vector>

#include "csvstats/stats.hpp"

namespace csvstats {

std::string format_report(const std::vector<ColumnStats>& columns);

}  // namespace csvstats
