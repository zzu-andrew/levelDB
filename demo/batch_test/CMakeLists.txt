cmake_minimum_required(VERSION 3.9)
# Keep the version below in sync with the one in db.h
# 这里一旦指定project那么下面所有的 ${PROJECT_SOURCE_DIR} 都会变成当前目录
#project(leveldb VERSION 1.23.0 LANGUAGES C CXX)


add_executable(write_batch
        write_batch_main.cpp
        write_batch_demo.cpp)

#target_link_libraries(test
#        PUBLIC leveldb)

target_include_directories(write_batch
        PUBLIC ${PROJECT_SOURCE_DIR}/include)

