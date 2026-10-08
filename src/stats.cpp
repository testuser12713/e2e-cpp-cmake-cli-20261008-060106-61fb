#include "csvstats/stats.hpp"

namespace csvstats {

// Skeleton stub. Ticket #4 replaces this with the real per-column computation.
StatsResult compute_stats(const Table& /*table*/,
                          const std::optional<std::string>& /*only_column*/) {
    StatsResult result;
    result.status = StatsStatus::Ok;
    return result;
}

}  // namespace csvstats
