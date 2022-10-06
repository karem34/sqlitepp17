#include "sqlitepp17.h"

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <string>

namespace type {
struct Person {
  std::string first_name;
  std::string last_name;
  uint age;
};
inline void from_sql(sqlitepp17::statement &stmt, Person &p, int idx) {
  p.first_name = stmt.get<std::string>(idx++);
  p.last_name = stmt.get<std::string>(idx++);
  p.age = stmt.get<int>(idx++);
};
inline void to_sql(sqlitepp17::statement &stmt, const Person &p, int idx) {
  stmt.bind(":first_name", p.first_name);
  stmt.bind(":last_name", p.last_name);
  stmt.bind(":age", p.age);
};
} // namespace type

int main() {
  using namespace type;
  using namespace sqlitepp17;
  try {
    // open db
    // can also open a file:
    // database db("data.db"); or read only:
    // database db("data.db", open_flags::READONLY);
    database db = database::memory();

    // execute query without parameters
    db.exec("CREATE TABLE IF NOT EXISTS people("
            "first_name text,"
            "last_name text,"
            "age integer"
            ")");

    // execute query with parameters
    db.exec("INSERT INTO people (first_name, last_name, age) VALUES (?, ?, ?)",
            "John", "Doe", 20);

    // can also use tuples or pairs
    db.exec("INSERT INTO people (first_name, last_name, age) VALUES (?, ?, ?)",
            std::tuple("Other", "Doe", 10));

    // and structs with 'to_sql' function
    Person jane = {"Jane", "Doe", 30};
    db.exec("INSERT INTO people (first_name, last_name, age)"
            "VALUES (:first_name, :last_name, :age)",
            jane);

    // select multiple rows and copy to a vector, identical to:
    // auto people = db.select<std::vector<Person>>("SELECT * FROM people");
    std::vector<Person> people;
    db.select(people, "SELECT * FROM people");
    for (const auto &p : people) {
      fmt::print("{} {} is {}\n", p.first_name, p.last_name, p.age);
    }

    // select one row and copy to a non-vector type
    // if type is not vector, it will only copy one row
    auto person =
        db.select<Person>("SELECT * FROM people WHERE first_name = ?", "Jane");
    fmt::print("{}'s age is {}\n", person.first_name, person.age);

    fmt::print("total age: {}\n",
               db.select<int>("SELECT SUM(age) FROM people"));

    // you can also use tuples or pairs
    auto tuple = db.select<std::tuple<std::string, std::string, int>>(
        "SELECT * FROM people WHERE age < ?", 25);
    fmt::print("{}'s age is {}\n", std::get<0>(tuple), std::get<2>(tuple));

    // also works with vectors
    auto names = db.select<std::vector<std::pair<std::string, std::string>>>(
        "SELECT first_name, last_name FROM people");
    fmt::print("{}\n", names);
    db.exec("INSERT INTO people (first_name, last_name, age) VALUES (?, ?, ?)",
            std::pair("Other", nullptr), 15);

    // null values also work (std::optional)
    std::vector<std::tuple<std::string, std::optional<std::string>>> nullable;
    db.select(nullable, "SELECT * FROM people");
    for (const auto &[first, last] : nullable) {
      fmt::print("{} {} a last name\n", first,
                 last.has_value() ? "has" : "doesn't have");
    }
    db.exec("DELETE FROM people WHERE last_name IS NULL");

    // you can also use lambdas
    int total_age = 0;
    db.query("SELECT age FROM People").exec([&](int age) { total_age += age; });
    fmt::print("total age is {}\n", total_age);

    db.exec("DELETE FROM people WHERE age < ?", 18);

    // you can also use statements
    std::string first_name_st, last_name_st;
    auto statement =
        db.query("SELECT first_name, last_name FROM people LIMIT 1")
            .to(first_name_st, last_name_st);
    fmt::print("{} {} is the first person\n", first_name_st, last_name_st);

    // this will throw if an error occurs, and return false if there are no more
    // rows, note that statement.to(), will execute a step and copy the output
    auto loop_statement = db.query("SELECT first_name FROM people");
    while (loop_statement.step()) {
      fmt::print("inside a loop, {}\n", loop_statement.get<std::string>());
    }
    return 0;
  } catch (const error &e) {
    fmt::print("{}\n", e.what());
    return -1;
  }
}
