//
// Created by wangyz38535 on 2023/12/5.
//
#include <iostream>

#include "write_batch_demo.h"

using namespace WRITE_BATCH_DEMO;


class HandlerDemo : public WRITE_BATCH_DEMO::WriteBatch::Handler {
public:
    ~HandlerDemo() override = default;

    void Put(const leveldb::Slice &key, const leveldb::Slice &value) override {
        std::cout << "put : " << key.ToString() << " : " << value.ToString() << std::endl;
    }

    void Delete(const leveldb::Slice &key) override {
        std::cout << "put : " << key.ToString() << std::endl;

    }
};


int main(int argc, char **argv) {


    int x = 12;

    WRITE_BATCH_DEMO::WriteBatch batch;
    HandlerDemo handler;

    //
    auto key = leveldb::Slice("xiaoming");
    auto value = leveldb::Slice("21");
    batch.Put(key, value);
    //batch.Put(leveldb::Slice("xiaohong"), leveldb::Slice("21"));
    //batch.Put(leveldb::Slice("wanger"), leveldb::Slice("21"));
    //batch.Put(leveldb::Slice("daxiong"), leveldb::Slice("21"));
    //
    //batch.Delete("xiaoming");
    //
    //batch.Iterate(&handler);




    return 0;
}


