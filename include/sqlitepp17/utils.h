#pragma once

#include <sqlite3.h>
#include <stdexcept>

namespace sqlitepp17 {

using int64 = sqlite3_int64;

class error : public std::runtime_error {
public:
  error(const int code) : std::runtime_error(sqlite3_errstr(code)){};
  error(const char *msg) : std::runtime_error(msg){};
};

inline void check(const int res) {
  if (res != SQLITE_DONE && res != SQLITE_OK && res != SQLITE_ROW)
    throw error(res);
};

inline void no_row() { throw error("no row"); }

} // namespace sqlitepp17