#pragma once

#include "statement.h"

namespace sqlitepp17 {

enum class open_flags {
  READONLY = SQLITE_OPEN_READONLY,
  READWRITE = SQLITE_OPEN_READWRITE,
  CREATE = SQLITE_OPEN_CREATE,
  URI = SQLITE_OPEN_URI,
  MEMORY = SQLITE_OPEN_MEMORY,
  NOMUTEX = SQLITE_OPEN_NOMUTEX,
  FULLMUTEX = SQLITE_OPEN_FULLMUTEX,
  SHAREDCACHE = SQLITE_OPEN_SHAREDCACHE,
  PRIVATECACHE = SQLITE_OPEN_PRIVATECACHE,
  NOFOLLOW = SQLITE_OPEN_NOFOLLOW
};
inline open_flags operator|(const open_flags &a, const open_flags &b) {
  return static_cast<open_flags>(static_cast<int>(a) | static_cast<int>(b));
}

struct database {
  database(const std::string &path,
           open_flags flags = open_flags::READWRITE | open_flags::CREATE) {
    sqlite3 *buf = nullptr;
    check(sqlite3_open_v2(path.data(), &buf, static_cast<int>(flags), nullptr));
    m_db = unique_sqlite(buf);
  };
  static database memory() { return std::move(database(":memory:")); }

  // non copyable
  database(const database &) = delete;
  database &operator=(const database &) = delete;
  database(database &&other) : m_db(std::move(other.m_db)) {}

  // new query (statement)
  template <typename... Args>
  inline statement query(const std::string &sql, Args &&...args) {
    statement st(sql, m_db.get());
    (st.bind(args), ...);
    return st;
  }

  // execute sql without return
  template <typename... Args>
  inline void exec(const std::string &sql, Args &&...args) {
    query(sql, args...).exec();
  }

  // execute sql and copy the result to 'out'
  // 'out' must provite a 'from_sql' function
  template <typename T, typename... Args>
  inline void select(T &out, const std::string &sql, Args &&...args) {
    query(sql, args...).to(out);
  }

  // execute sql and return the result
  // 'T' must provite a 'from_sql' function
  template <typename T, typename... Args>
  inline T select(const std::string &sql, Args &&...args) {
    T out;
    query(sql, args...).to(out);
    return out;
  }

private:
  struct closer {
    void operator()(sqlite3 *db) const { sqlite3_close_v2(db); }
  };
  using unique_sqlite = std::unique_ptr<sqlite3, closer>;
  unique_sqlite m_db;
};

} // namespace sqlitepp17
