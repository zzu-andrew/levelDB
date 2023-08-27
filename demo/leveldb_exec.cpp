//
// Created by andrew on 2021/10/16.
//
#include <iostream>
#include <cassert>
#include "leveldb/write_batch.h"
#include "leveldb/db.h"

using namespace std;


int main(int argc, char *argv[]) {

    // 打开一个内存数据库
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
    assert(status.ok());

    // 数据库的读写
    std::string value = "value";
    std::string key1 = "key1";

    auto s = db->Put(leveldb::WriteOptions(), key1, value);

    for (int i = 0; i < 100; ++i) {
        std::string key = to_string(i);
        std::string valStr = to_string(i + 2);
        s = db->Put(leveldb::WriteOptions(), key, valStr);
        if (s.ok()) {
            std::cout << valStr << std::endl;
        } else {
            std::cout << "put failed" << std::endl;
        }
    }

    std::string valData;
    s = db->Get(leveldb::ReadOptions(), key1, &valData);
    if (s.ok()) {
        std::cout << valData << std::endl;
    } else {
        std::cout << "Get key1 failed" << std::endl;
    }
//    if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);


    // 若是上述操作在Delete之前系统挂掉了，会导致value存储到多个key下
    // 若是想多个步骤同时完成可以使用原子操作(事务)
    std::string value2 = "value2";
    std::string key2 = "key2";
    s = db->Get(leveldb::ReadOptions(), key1, &value);
    if (s.ok()) {
        leveldb::WriteBatch batch;
        batch.Delete(key1);
        batch.Put(key2, value);
        s = db->Write(leveldb::WriteOptions(), &batch);
        if (s.ok()) {

        } else {
            std::cout << "put failed" << std::endl;
        }
    }

    // 数据库不使用的时候记得及时清除
    delete db;

    return 0;
}