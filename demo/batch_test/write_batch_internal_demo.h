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

    };

}

#endif //LEVELDB_WRITE_BATCH_INTERNAL_DEMO_H
