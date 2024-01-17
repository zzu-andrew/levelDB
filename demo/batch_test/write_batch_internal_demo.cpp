//
// Created by wangyz38535 on 2023/12/5.
//

#include <cassert>
#include "write_batch_internal_demo.h"

using namespace WRITE_BATCH_DEMO;

uint32_t WriteBatchInternal::Count(const WriteBatch *batch) {
    //   8-byte sequence number
    const auto *const buffer =  reinterpret_cast<const uint8_t *>(batch->rep_.data() + 8);

    // Recent clang and gcc optimize this to a single mov / ldr instruction.
    return (static_cast<uint32_t>(buffer[0])) |
            (static_cast<uint32_t>(buffer[1]) << 8) |
            (static_cast<uint32_t>(buffer[2]) << 16) |
            (static_cast<uint32_t>(buffer[3]) << 24);
}

void WriteBatchInternal::SetCount(WriteBatch *batch, uint32_t n) {
    // 前面8byte sequence number
    // 这里和上边的去比啊，这里使用[]取值是因为[]取出的非const 引用
    auto *const buffer =  reinterpret_cast<uint8_t *>(&batch->rep_[8]);
    // 将uint32_t类型数据每8个字节存储到一个chat里面
    buffer[0] = static_cast<uint8_t>(n);
    buffer[1] = static_cast<uint8_t>(n >> 8);
    buffer[2] = static_cast<uint8_t>(n >> 16);
    buffer[3] = static_cast<uint8_t>(n >> 24);
}

leveldb::SequenceNumber WriteBatchInternal::Sequence(const WriteBatch *batch) {
    const auto *const buffer = reinterpret_cast<const uint8_t *>(batch->rep_.data());

    // Recent clang and gcc optimize this to a single mov / ldr instruction.
    return (static_cast<uint64_t>(buffer[0])) |
        (static_cast<uint64_t>(buffer[1]) << 8) |
        (static_cast<uint64_t>(buffer[2]) << 16) |
        (static_cast<uint64_t>(buffer[3]) << 24) |
        (static_cast<uint64_t>(buffer[4]) << 32) |
        (static_cast<uint64_t>(buffer[5]) << 40) |
        (static_cast<uint64_t>(buffer[6]) << 48) |
        (static_cast<uint64_t>(buffer[7]) << 56);
}

void WriteBatchInternal::SetSequence(WriteBatch *batch, leveldb::SequenceNumber seq) {
    auto *const buffer = reinterpret_cast<uint8_t *>(&batch->rep_[0]);

    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(seq);
    buffer[1] = static_cast<uint8_t>(seq >> 8);
    buffer[2] = static_cast<uint8_t>(seq >> 16);
    buffer[3] = static_cast<uint8_t>(seq >> 24);
    buffer[4] = static_cast<uint8_t>(seq >> 32);
    buffer[5] = static_cast<uint8_t>(seq >> 40);
    buffer[6] = static_cast<uint8_t>(seq >> 48);
    buffer[7] = static_cast<uint8_t>(seq >> 56);
}

void WriteBatchInternal::SetContents(WriteBatch *batch, const leveldb::Slice &contents) {
    assert(contents.size() >= kHeader);
    batch->rep_.assign(contents.data(), contents.size());
}

void WriteBatchInternal::Append(WriteBatch *dst, const WriteBatch *src) {
    SetCount(dst, Count(dst) + Count(src));
    assert(src->rep_.size() >= kHeader);
    // 两个Writebatch合并时，需要剔除多于的头部信息， 4字节长度 + 8字节序列号
    dst->rep_.append(src->rep_.data() + kHeader, src->rep_.size() - kHeader);
}

// batch 通过迭代器接口调用的的事FraudIterate，而FraudIterate对象反过来调用lpFraud将具体的数据转移到lpFraud指向的对象里面
leveldb::Status WriteBatchInternal::InsertInfo(const WriteBatch *batch, void * lpFraud) {
    // FraudIterate 相当于一个壳，如果类比设计模式，这里用的就 桥接模式（Bridge Pattern）
    FraudIterate inserter(lpFraud);
    return batch->Iterate(&inserter);
}
