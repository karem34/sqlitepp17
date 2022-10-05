#pragma once

#include "utils.h"

#include <memory>
#include <string>
#include <vector>

namespace sqlitepp17 {

struct statement;
// for types to be used in this library
// they need to implement 'from_sql' and 'to_sql' functions
// the library provides implementations for standard types
template <typename T> inline void from_sql(statement &, T &, const int);
template <typename T> inline void to_sql(statement &, const T &, int);

struct statement {
  statement(const std::string &sql, sqlite3 *db) : m_db(db) {
    sqlite3_stmt *buf;
    check(sqlite3_prepare_v2(m_db, sql.data(), sql.length(), &buf, nullptr));

    m_stmt = unique_stmt(buf);
  };

  statement(const statement &) = delete;
  statement &operator=(const statement &) = delete;
  statement(statement &&other)
      : m_db(std::move(other.m_db)), m_stmt(std::move(other.m_stmt)),
        m_bind_idx(other.m_bind_idx) {}

  // bind next parameter
  template <typename T> inline statement &&bind(const T &value) {
    bind(m_bind_idx++, value);
    return std::move(*this);
  }
  // bind parameter with an index
  template <typename T> inline statement &&bind(int idx, const T &value) {
    to_sql(*this, value, idx++);
    return std::move(*this);
  }
  // bind parameter (:VVV) to a value , do nothing if parameter is not found
  template <typename T>
  inline statement &&bind(const std::string &col, const T &value) {
    const int idx = sqlite3_bind_parameter_index(m_stmt.get(), col.data());
    if (idx) {
      bind(idx, value);
      m_bind_idx = idx + 1;
    }
    return std::move(*this);
  }
  // does not execute statement
  template <typename T> inline T get(const int idx = 0) {
    T val;
    from_sql(*this, val, idx);
    return val;
  }
  // this will clear the vector
  template <typename T> inline statement &&to(std::vector<T> &out) {
    out.clear();
    while (step())
      out.push_back(get<T>());
    return std::move(*this);
  };
  template <typename T> inline statement &&to(T &out) {
    if (!step())
      no_row();
    out = get<T>();
    return std::move(*this);
  }

  inline statement &&exec() {
    while (step()) {
    }
    return std::move(*this);
  }
  template <typename Func> inline statement &&exec(Func &&func) {
    using args = typename function_traits<decltype(&Func::operator())>::args;
    while (step()) {
      std::apply(func, get<args>());
    }
    return std::move(*this);
  }

  inline bool step() {
    int res = sqlite3_step(m_stmt.get());
    if (res == SQLITE_ROW)
      return true;
    if (res == SQLITE_DONE)
      return false;
    check(res);
    return false;
  }

  inline statement &&reset() {
    check(sqlite3_reset(m_stmt.get()));
    check(sqlite3_clear_bindings(m_stmt.get()));
    m_bind_idx = 1;
    return std::move(*this);
  }
  inline bool readonly() { return sqlite3_stmt_readonly(m_stmt.get()); }
  inline void decrement_next() { m_bind_idx = std::max(m_bind_idx - 1, 1); }
  inline int bind_idx() const { return m_bind_idx; }
  inline std::string sql() { return sqlite3_expanded_sql(m_stmt.get()); }
  inline int columns() { return sqlite3_column_count(m_stmt.get()); }
  inline int column_type(const int idx) {
    return sqlite3_column_type(m_stmt.get(), idx);
  }

  // these call the sqlite3 functions, use 'bind' instead
  inline void bind_impl(const int idx, const int value) {
    check(sqlite3_bind_int(m_stmt.get(), idx, value));
  }
  inline void bind_impl(const int idx, const std::string &value) {
    check(sqlite3_bind_text(m_stmt.get(), idx, value.data(), value.length(),
                            SQLITE_TRANSIENT));
  }
  inline void bind_impl(const int idx, const std::nullptr_t &) {
    check(sqlite3_bind_null(m_stmt.get(), idx));
  }

  // these call the sqlite3 functions, use 'get' instead
  inline int get_impl_i(const int idx) {
    return sqlite3_column_int(m_stmt.get(), idx);
  };
  inline bool get_impl_b(const int idx) {
    return sqlite3_column_int(m_stmt.get(), idx);
  }
  inline std::string get_impl_s(const int idx) {
    return reinterpret_cast<const char *>(
        sqlite3_column_text(m_stmt.get(), idx));
  }
  inline const void *get_impl_blob(const int idx) {
    return sqlite3_column_blob(m_stmt.get(), idx);
  }
  inline double get_impl_d(const int idx) {
    return sqlite3_column_double(m_stmt.get(), idx);
  }
  inline int64 get_impl_i64(const int idx) {
    return sqlite3_column_int64(m_stmt.get(), idx);
  }

private:
  template <typename T>
  struct function_traits : function_traits<decltype(&T::operator())> {};
  template <typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const> {
    using args = std::tuple<Args...>;
  };
  struct deleter {
    void operator()(sqlite3_stmt *stmt) const { sqlite3_finalize(stmt); }
  };
  using unique_stmt = std::unique_ptr<sqlite3_stmt, deleter>;

  unique_stmt m_stmt;
  sqlite3 *m_db;
  int m_bind_idx{1};
};

} // namespace sqlitepp17
