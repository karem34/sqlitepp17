# SQLitePP17

SQLite header-only wrapper for C++17, inspired by [sqlx](https://github.com/jmoiron/sqlx) and [nlohmann/json](https://github.com/nlohmann/json).

## Basic Usage

Check out `example.cpp` for a working example.

```cpp
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
        std::pair("Other", "Doe"), 10);
```

### Select

```cpp
// using a vector
std::vector<Person> people;
db.select(people, "SELECT * FROM people");

// single row
auto person =
    db.select<Person>("SELECT * FROM people WHERE first_name = ?", "Jane");

// lambdas
int total_age = 0;
db.query("SELECT age FROM People").exec([&](int age) { total_age += age; });
```

### Custom Types

```cpp
struct Person {
  std::string first_name;
  std::string last_name;
  uint age;
};
template <>
inline void sqlitepp17::from_sql(statement &stmt, Person &p, int idx) {
  p.first_name = stmt.get<std::string>(idx++);
  p.last_name = stmt.get<std::string>(idx++);
  p.age = stmt.get<int>(idx++);
};
template <>
inline void sqlitepp17::to_sql(statement &stmt, const Person &p, int idx) {
  stmt.bind(":first_name", p.first_name);
  stmt.bind(":last_name", p.last_name);
  stmt.bind(":age", p.age);
};

Person jane = {"Jane", "Doe", 30};
db.exec("INSERT INTO people (first_name, last_name, age)"
        "VALUES (:first_name, :last_name, :age)",
        jane);

}
```

### Null Values

```cpp
// null values also work (using std::optional)
std::vector<std::tuple<std::string, std::optional<std::string>>> nullable;

db.select(nullable, "SELECT first_name, last_name FROM people");

for (const auto &[first, last] : nullable) {
    fmt::print("{} {} a last name\n", first,
                last.has_value() ? "has" : "doesn't have");
}

db.exec("DELETE FROM people WHERE last_name IS NULL");
```

## Building and Installing

SQLitePP17 uses CMake for building.

```bash
cmake . -B build # to build and:
cd build && sudo make install # to install
```

You can also use the files in the `include` directory directly.

To compile the example, use:

```bash
g++ example.cpp -lsqlite3 -lfmt # or:
clang example.cpp -lsqlite3 -lfmt -lstdc++ -std=c++17
```

## Dependencies

This library depends only on sqlite3 and a C++ compiler supporting C++17 or newer.

The example requires [fmtlib](https://fmt.dev).

## License

MIT License
