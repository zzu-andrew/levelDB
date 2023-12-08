cmake_minimum_required(VERSION 3.9)
# Keep the version below in sync with the one in db.h



add_executable(leveldb_exec leveldb_exec.cpp)

target_link_libraries(leveldb_exec
        PUBLIC leveldb)

target_include_directories(leveldb_exec
        PUBLIC ${PROJECT_SOURCE_DIR}/../include)


add_executable(test test.cpp)

target_link_libraries(test
        PUBLIC leveldb)

target_include_directories(test
        PUBLIC ${PROJECT_SOURCE_DIR}/include)


# 添加各个算法测试代码
add_subdirectory(batch_test)
