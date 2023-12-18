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
 *
 * */
namespace WRITE_BATCH_DEMO {

    // WriteBatch header has an 8-byte sequence number followed by a 4-byte count.
    static const size_t kHeader = 12;

    class WriteBatch {
    public:
        class LEVELDB_EXPORT Handler {
        public:
            virtual ~Handler();

            virtual void Put(const leveldb::Slice &key, const leveldb::Slice &value) = 0;

            virtual void Delete(const leveldb::Slice &key) = 0;
        };

        WriteBatch();

        // Intentionally copyable.
        WriteBatch(const WriteBatch &) = default;

        WriteBatch &operator=(const WriteBatch &) = default;

        ~WriteBatch();

        // Store the mapping "key->value" in the database.
        void Put(const leveldb::Slice &key, const leveldb::Slice &value);

        // If the database contains a mapping for "key", erase it.  Else do nothing.
        void Delete(const leveldb::Slice &key);

        // Clear all updates buffered in this batch.
        void Clear();

        // The size of the database changes caused by this batch.
        //
        // This number is tied to implementation details, and may change across
        // releases. It is intended for LevelDB usage metrics.
        size_t ApproximateSize() const;

        // Copies the operations in "source" to this batch.
        //
        // This runs in O(source size) time. However, the constant factor is better
        // than calling Iterate() over the source batch with a Handler that replicates
        // the operations into this batch.
        void Append(const WriteBatch &source);

        // Support for iterating over the contents of a batch.
        leveldb::Status Iterate(Handler *handler) const;

    private:
        friend class WriteBatchInternal;

        std::string rep_;  // See comment in write_batch.cc for the format of rep_
    };

}


#endif //LEVELDB_WRITE_BATCH_DEMO_H
