// Tests for the csvstats_core CSV reader: quoting, embedded delimiters,
// escaped quotes, CRLF/LF endings, UTF-8 BOM, blank lines, surrounding
// whitespace, empty/header-only/missing files.

#include <csvstats/csv.hpp>

#include <filesystem>
#include <fstream>
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

std::string render(const std::vector<std::string>& fields) {
    std::string out = "[";
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) {
            out += ", ";
        }
        out += "'" + fields[i] + "'";
    }
    out += "]";
    return out;
}

void check_fields(const std::vector<std::string>& got,
                  const std::vector<std::string>& want,
                  const std::string& what) {
    if (got != want) {
        std::cerr << "FAIL: " << what << " expected " << render(want) << " got "
                  << render(got) << '\n';
        ++failures;
    }
}

std::filesystem::path make_temp_dir() {
    std::error_code ec;
    std::filesystem::path base = std::filesystem::temp_directory_path(ec);
    if (ec) {
        base = ".";
    }
    for (int i = 0; i < 100000; ++i) {
        const std::filesystem::path dir =
            base / ("csvstats_csv_test_" + std::to_string(i));
        std::error_code create_ec;
        if (std::filesystem::create_directory(dir, create_ec) && !create_ec) {
            return dir;
        }
    }
    return base;
}

std::filesystem::path write_file(const std::filesystem::path& dir,
                                 const std::string& name,
                                 const std::string& content) {
    const std::filesystem::path path = dir / name;
    std::ofstream out(path, std::ios::binary);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return path;
}

}  // namespace

int main() {
    // --- parse_line: splitting and whitespace ---
    check_fields(csvstats::parse_line("a,b,c", ','), {"a", "b", "c"},
                 "parse_line splits on the delimiter");
    check_fields(csvstats::parse_line("  a , b  ", ','), {"a", "b"},
                 "parse_line trims whitespace around unquoted fields");
    check_fields(csvstats::parse_line("New York, 42 ", ','), {"New York", "42"},
                 "parse_line keeps whitespace inside a field");
    check_fields(csvstats::parse_line("a,", ','), {"a", ""},
                 "parse_line keeps a trailing empty field");

    // --- parse_line: quoting ---
    check_fields(csvstats::parse_line("\"a,b\",c", ','), {"a,b", "c"},
                 "parse_line keeps a delimiter inside quotes");
    check_fields(csvstats::parse_line("\"he said \"\"hi\"\"\",x", ','),
                 {"he said \"hi\"", "x"},
                 "parse_line unescapes doubled quotes");
    check_fields(csvstats::parse_line("\"\"", ','), {""},
                 "parse_line reads an empty quoted field");
    check_fields(csvstats::parse_line(" \"a\" , b", ','), {"a", "b"},
                 "parse_line trims whitespace around a quoted field");

    // --- parse_line: line endings and blank lines ---
    check_fields(csvstats::parse_line("a,b\r", ','), {"a", "b"},
                 "parse_line strips a trailing CR (CRLF)");
    check(csvstats::parse_line("", ',').empty(),
          "parse_line treats an empty line as blank");
    check(csvstats::parse_line("   ", ',').empty(),
          "parse_line treats a whitespace-only line as blank");
    check(csvstats::parse_line("\r", ',').empty(),
          "parse_line treats a bare CR line as blank");
    check(csvstats::parse_line("\t", '\t').size() == 2,
          "parse_line keeps tab-delimiter fields on a tab-only line");

    const std::filesystem::path dir = make_temp_dir();

    // --- parse_file: normal CRLF file ---
    {
        const std::filesystem::path path =
            write_file(dir, "normal.csv", "name,value\r\nAlice,1\r\nBob,2\r\n");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::Ok,
              "parse_file reads a valid CRLF file");
        check_fields(result.table.header, {"name", "value"},
                     "parse_file stores the header");
        check(result.table.rows.size() == 2,
              "parse_file stores every data row");
        if (result.table.rows.size() == 2) {
            check_fields(result.table.rows[0], {"Alice", "1"},
                         "parse_file reads the first row");
            check_fields(result.table.rows[1], {"Bob", "2"},
                         "parse_file reads the second row");
        }
    }

    // --- parse_file: BOM, blank lines, quoting ---
    {
        const std::filesystem::path path = write_file(
            dir, "bom.csv",
            "\xEF\xBB\xBF" "id,note\n\n1,\"a,b\"\n\n2,\"x\"\"y\"\n");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::Ok,
              "parse_file handles BOM and blank lines");
        check_fields(result.table.header, {"id", "note"},
                     "parse_file strips the BOM from the header");
        check(result.table.rows.size() == 2,
              "parse_file skips blank lines instead of making rows");
        if (result.table.rows.size() == 2) {
            check_fields(result.table.rows[0], {"1", "a,b"},
                         "parse_file keeps a delimiter inside quoted data");
            check_fields(result.table.rows[1], {"2", "x\"y"},
                         "parse_file unescapes doubled quotes in data");
        }
    }

    // --- parse_file: custom delimiter ---
    {
        const std::filesystem::path path =
            write_file(dir, "semi.csv", "a;b\n1;2\n");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ';');
        check(result.status == csvstats::ParseStatus::Ok,
              "parse_file honours the requested delimiter");
        check_fields(result.table.header, {"a", "b"},
                     "parse_file splits the header on the delimiter");
    }

    // --- parse_file: empty file ---
    {
        const std::filesystem::path path = write_file(dir, "empty.csv", "");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::EmptyFile,
              "parse_file reports a zero-byte file as EmptyFile");
        check(!result.message.empty() &&
                  result.message.find("empty.csv") != std::string::npos,
              "parse_file empty message names the file");
    }

    // --- parse_file: header without data rows ---
    {
        const std::filesystem::path path =
            write_file(dir, "header_only.csv", "a,b,c\n");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::EmptyFile,
              "parse_file reports a header-only file as EmptyFile");
        check(result.message.find("header_only.csv") != std::string::npos,
              "parse_file header-only message names the file");
    }

    // --- parse_file: only blank lines ---
    {
        const std::filesystem::path path =
            write_file(dir, "blank_only.csv", "\n\n   \n");
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::EmptyFile,
              "parse_file reports a file of blank lines as EmptyFile");
    }

    // --- parse_file: missing file ---
    {
        const std::filesystem::path path = dir / "does_not_exist.csv";
        const csvstats::ParseResult result = csvstats::parse_file(path.string(), ',');
        check(result.status == csvstats::ParseStatus::FileMissing,
              "parse_file reports a missing file as FileMissing");
        check(!result.message.empty() &&
                  result.message.find("does_not_exist.csv") != std::string::npos,
              "parse_file missing message names the file");
    }

    std::error_code remove_ec;
    std::filesystem::remove_all(dir, remove_ec);

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "test_csv: all checks passed\n";
    return 0;
}
