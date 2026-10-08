#include "csvstats/csv.hpp"

namespace csvstats {

// Skeleton stub. Ticket #3 replaces this with the real CSV reader
// (quoting, BOM, CRLF, blank lines, whitespace trimming).
ParseResult parse_file(const std::string& /*path*/, char /*delimiter*/) {
    ParseResult result;
    result.status = ParseStatus::Ok;
    return result;
}

// Skeleton stub. Ticket #3 replaces this with field splitting.
std::vector<std::string> parse_line(const std::string& /*line*/, char /*delimiter*/) {
    return std::vector<std::string>{};
}

}  // namespace csvstats
