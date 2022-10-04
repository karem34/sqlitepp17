cmake_minimum_required(VERSION 3.14)

project(sqlitepp17 LANGUAGES CXX)

add_library(sqlitepp17 INTERFACE)
add_library(sqlitepp17::sqlitepp17 ALIAS sqlitepp17)

target_compile_features(sqlitepp17 INTERFACE cxx_std_117)

target_include_directories(sqlitepp17
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

install(TARGETS sqlitepp17 EXPORT sqlitepp17-targets)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

find_package(SQLite3 REQUIRED)

target_link_libraries(sqlitepp17 INTERFACE SQLite::SQLite3)
