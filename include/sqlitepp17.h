#pragma once

#include "./sqlitepp17/database.h"

#include <optional>
#include <tuple>

namespace sqlitepp17 {

// these are 'from_sql' and 'to_sql' implementations for various standard types
inline void from_sql(statement &stmt, std::string &value, const int idx) {
  value = stmt.get_impl_s(idx);
};
inline void from_sql(statement &stmt, int &value, const int idx) {
  value = stmt.get_impl_i(idx);
};
inline void from_sql(statement &stmt, double &value, const int idx) {
  value = stmt.get_impl_d(idx);
};
inline void from_sql(statement &stmt, const void *&value, const int idx) {
  value = stmt.get_impl_blob(idx);
};

template <typename... T>
inline void from_sql(statement &stmt, std::tuple<T...> &tuple, int idx) {
  std::apply(
      [&](auto &&...args) {
        ((args = stmt.get<std::remove_reference_t<decltype(args)>>(idx++)),
         ...);
      },
      tuple);
};

template <typename T1, typename T2>
inline void from_sql(statement &stmt, std::pair<T1, T2> &pair, int idx) {
  pair.first = stmt.get<T1>(idx++);
  pair.second = stmt.get<T2>(idx++);
};

template <typename T>
inline void from_sql(statement &stmt, std::optional<T> &opt, int idx) {
  if (stmt.column_type(idx) == SQLITE_NULL)
    opt = std::nullopt;
  else
    opt = stmt.get<T>(idx);
};

template <typename T>
inline void to_sql(statement &stmt, const T &value, const int idx) {
  stmt.bind_impl(idx, value);
};

template <typename... T>
inline void to_sql(statement &stmt, const std::tuple<T...> &tuple, const int) {
  // this is necessary because tuple doesn't bind anything
  stmt.decrement_next();
  std::apply([&](auto &&...args) { (stmt.bind(args), ...); }, tuple);
};

template <typename T1, typename T2>
inline void to_sql(statement &stmt, const std::pair<T1, T2> &pair, const int) {
  stmt.decrement_next();
  stmt.bind(pair.first);
  stmt.bind(pair.second);
};

} // namespace sqlitepp17
