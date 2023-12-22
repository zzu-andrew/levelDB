//
// Created by wangyz38535 on 2023/12/5.
//

#include "write_batch_demo.h"

using namespace WRITE_BATCH_DEMO;

WriteBatch::WriteBatch() {
    Clear();
}

WriteBatch::~WriteBatch() = default;

void WriteBatch::Put(const leveldb::Slice &key, const leveldb::Slice &value) {

}

void WriteBatch::Delete(const leveldb::Slice &key) {

}

void WriteBatch::Clear() {
    // 清空对象
    rep_.clear();
    // 预先设置为指定的头长度
    rep_.resize(kHeader);
}

size_t WriteBatch::ApproximateSize() const {
    return rep_.size();
}

void WriteBatch::Append(const WriteBatch &source) {

}

leveldb::Status WriteBatch::Iterate(WriteBatch::Handler *handler) const {
    return leveldb::Status();
}
