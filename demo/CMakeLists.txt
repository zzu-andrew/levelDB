cmake_minimum_required(VERSION 3.9)
# Keep the version below in sync with the one in db.h
project(leveldb VERSION 1.23.0 LANGUAGES C CXX)



add_executable(leveldb_exec leveldb_exec.cpp)

target_link_libraries(leveldb_exec
        PUBLIC leveldb)

target_include_directories(leveldb_exec
        PUBLIC ${PROJECT_SOURCE_DIR}/../include)


add_executable(dataTest test.cpp)
