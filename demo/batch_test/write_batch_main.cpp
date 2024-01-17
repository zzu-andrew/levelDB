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
        std::cout << "Delete : " << key.ToString() << std::endl;

    }
};

int main(int argc, char **argv) {

    WRITE_BATCH_DEMO::WriteBatch batch;
    WRITE_BATCH_DEMO::WriteBatch batch1;
    HandlerDemo handler;

    //
    auto key = leveldb::Slice("xiaoming");
    auto value = leveldb::Slice("21");
    batch.Put(key, value);
    batch.Put(leveldb::Slice("xiaohong"), leveldb::Slice("21"));
    batch.Put(leveldb::Slice("wanger"), leveldb::Slice("22"));
    batch.Put(leveldb::Slice("daxiong"), leveldb::Slice("24"));

    batch1.Put(leveldb::Slice("Batch1ForTest"), leveldb::Slice("18"));

    batch.Delete("xiaoming");

    std::cout << batch.ApproximateSize() << std::endl;
    batch.Append(batch1);
    std::cout << batch.ApproximateSize() << std::endl;

    std::cout << batch.Iterate(&handler).ToString() << std::endl;

    return 0;
}


