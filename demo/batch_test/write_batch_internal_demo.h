//
// Created by wangyz38535 on 2023/12/5.
//

#ifndef LEVELDB_WRITE_BATCH_INTERNAL_DEMO_H
#define LEVELDB_WRITE_BATCH_INTERNAL_DEMO_H
#include "db/dbformat.h"
#include "write_batch_demo.h"

namespace WRITE_BATCH_DEMO {

    // WriteBatchInternalDemo 提供static 函数用来处理 WriterBatchDemo，这里的函数是公用的
    // 但是我们又不想放到WriterBatchDemo的public里面，可以考虑使用这种方式
    class WriteBatchInternal {
    public:
        // 返回入参WriteBatch存储的事务个数
        static uint32_t Count(const WriteBatch* batch);
        // 设置当前writeBatch中包含事务的个数
        static void SetCount(WriteBatch* batch, uint32_t n);
        // 将batch开头的序列号返回
        static leveldb::SequenceNumber Sequence(const WriteBatch* batch);
        // 将一个特殊的数字，作为序列号存储在batch的开头
        static void SetSequence(WriteBatch* batch, leveldb::SequenceNumber seq);

        static leveldb::Slice Contents(const WriteBatch* batch) {
            return leveldb::Slice(batch->rep_);
        }

        static size_t ByteSize(const WriteBatch* batch) {
            return batch->rep_.size();
        }

        static void SetContents(WriteBatch* batch, const leveldb::Slice& contents);
        // 仿写， 不需要进行真正的数据存储，这里去除后面的数据表项
        static leveldb::Status InsertInfo(const WriteBatch* batch);

        static void Append(WriteBatch* dst, const WriteBatch* src);

    };

}

#endif //LEVELDB_WRITE_BATCH_INTERNAL_DEMO_H
