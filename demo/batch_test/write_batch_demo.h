//
// Created by wangyz38535 on 2023/12/5.
//

#ifndef LEVELDB_WRITE_BATCH_DEMO_H
#define LEVELDB_WRITE_BATCH_DEMO_H

#include <algorithm>
#include <string>
#include "leveldb/slice.h"
#include "leveldb/export.h"
#include "leveldb/status.h"

/*
 * 对write_bach 进行重新实现，一般理解里面的实现原理
 * */
namespace WRITE_BATCH_DEMO {

    // WriteBatch header has an 8-byte sequence number followed by a 4-byte count.
    static const size_t kHeader = 12;

    class WriteBatch {
    public:
        class LEVELDB_EXPORT Handler {
        public:
            virtual ~Handler() = default;

            virtual void Put(const leveldb::Slice &key, const leveldb::Slice &value) = 0;

            virtual void Delete(const leveldb::Slice &key) = 0;
        };

        WriteBatch();

        // 支持copy构造函数
        WriteBatch(const WriteBatch &) = default;
        // 支持赋值操作
        WriteBatch &operator=(const WriteBatch &) = default;

        ~WriteBatch();

        // Put 将Key-value形式的值存储在数据库中
        void Put(const leveldb::Slice &key, const leveldb::Slice &value);
        // 当某个值不想要时,按照key值将对应的键值对删除
        void Delete(const leveldb::Slice &key);
        // 将 batch中所有的缓存清理，并预留出长度和序列号字段足够的长度
        void Clear();
        // 获取batch内部指标数据长度，也就是获取rep_的大小
        size_t ApproximateSize() const;
        // 当有多个batch想合并时，可以使用append将batch添加到另外衣蛾batch后面
        void Append(const WriteBatch &source);
        // 对batch进行迭代，这样能够按照类型取出所有batch里面的值
        leveldb::Status Iterate(Handler *handler) const;

    private:
        friend class WriteBatchInternal;

        std::string rep_;  // See comment in write_batch.cc for the format of rep_
    };

}


#endif //LEVELDB_WRITE_BATCH_DEMO_H
