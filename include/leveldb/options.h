// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <cstddef>

#include "leveldb/export.h"

namespace leveldb {

    class Cache;

    class Comparator;

    class Env;

    class FilterPolicy;

    class Logger;

    class Snapshot;

// DB contents are stored in a set of blocks, each of which holds a
// sequence of key,value pairs.  Each block may be compressed before
// being stored in a file.  The following enum describes which
// compression method (if any) is used to compress a block.
    enum CompressionType {
        // NOTE: do not change the values of existing entries, as these are
        // part of the persistent format on disk.
        kNoCompression = 0x0,
        kSnappyCompression = 0x1
    };

// Options 通过传入给 DB::OPen 来控制数据库行为
    struct LEVELDB_EXPORT Options {
        // 使用默认值构造Options
        Options();

        // Comparator 用来定义keys值在table中的排序方式
        // Default: 默认提供的对比工具是按照字典顺序进行排序的，也就是字典中字幕的排列顺序进行排序
        // 需要用户保证所有打开数据的对比工具都是一致的
        const Comparator *comparator;

        // 如果设置为true，当数据库丢失时会创建一个新的数据库
        bool create_if_missing = false;

        // 如果设置为true，当同名数据库已经存在就直接报错
        bool error_if_exists = false;

        // 如果设置为true，将会实现对正在设置的数据进行检查，如果检测到任何错误将会提前停止
        // 这可能会导致不可预料的异常，例如：一个损坏的DB表项，可能造成大量的DB表项不可读
        // 甚至导致整个数据库不可打开
        bool paranoid_checks = false;

        // 一些和使用的平台相关的操作，如：读写文件操作
        // Default: Env::Default()
        Env *env;

        // 如果该句柄非空，DB中发生的任何错误都会记录到info_log句柄中，日志会在DB同样的目录里面创建
        // 一个日志文件用来存储日志记录
        Logger *info_log = nullptr;

        // -------------------
        // Parameters that affect performance

        // 写缓存大小，越大速度越快，在内存中存储的数据是无序的，当进行落盘的时候需要转化为有序
        // 状态。一个大的写缓存可能会导致比较长的数据库恢复时间
        size_t write_buffer_size = 4 * 1024 * 1024;

        // Number of open files that can be used by the DB.  You may need to
        // increase this if your database has a large working set (budget
        // one open file per 2MB of working set).
        int max_open_files = 1000;

        // Control over blocks (user data is stored in a set of blocks, and
        // a block is the unit of reading from disk).

        // If non-null, use the specified cache for blocks.
        // If null, leveldb will automatically create and use an 8MB internal cache.
        Cache *block_cache = nullptr;

        // Approximate size of user data packed per block.  Note that the
        // block size specified here corresponds to uncompressed data.  The
        // actual size of the unit read from disk may be smaller if
        // compression is enabled.  This parameter can be changed dynamically.
        size_t block_size = 4 * 1024;

        // Number of keys between restart points for delta encoding of keys.
        // This parameter can be changed dynamically.  Most clients should
        // leave this parameter alone.
        int block_restart_interval = 16;

        // Leveldb will write up to this amount of bytes to a file before
        // switching to a new one.
        // Most clients should leave this parameter alone.  However if your
        // filesystem is more efficient with larger files, you could
        // consider increasing the value.  The downside will be longer
        // compactions and hence longer latency/performance hiccups.
        // Another reason to increase this parameter might be when you are
        // initially populating a large database.
        size_t max_file_size = 2 * 1024 * 1024;

        // Compress blocks using the specified compression algorithm.  This
        // parameter can be changed dynamically.
        //
        // Default: kSnappyCompression, which gives lightweight but fast
        // compression.
        //
        // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
        //    ~200-500MB/s compression
        //    ~400-800MB/s decompression
        // Note that these speeds are significantly faster than most
        // persistent storage speeds, and therefore it is typically never
        // worth switching to kNoCompression.  Even if the input data is
        // incompressible, the kSnappyCompression implementation will
        // efficiently detect that and will switch to uncompressed mode.
        CompressionType compression = kSnappyCompression;

        // EXPERIMENTAL: If true, append to existing MANIFEST and log files
        // when a database is opened.  This can significantly speed up open.
        //
        // Default: currently false, but may become true later.
        bool reuse_logs = false;

        // If non-null, use the specified filter policy to reduce disk reads.
        // Many applications will benefit from passing the result of
        // NewBloomFilterPolicy() here.
        const FilterPolicy *filter_policy = nullptr;
    };

// Options that control read operations
    struct LEVELDB_EXPORT ReadOptions {
        ReadOptions() = default;

        // If true, all data read from underlying storage will be
        // verified against corresponding checksums.
        bool verify_checksums = false;

        // Should the data read for this iteration be cached in memory?
        // Callers may wish to set this field to false for bulk scans.
        bool fill_cache = true;

        // If "snapshot" is non-null, read as of the supplied snapshot
        // (which must belong to the DB that is being read and which must
        // not have been released).  If "snapshot" is null, use an implicit
        // snapshot of the state at the beginning of this read operation.
        const Snapshot *snapshot = nullptr;
    };

// Options that control write operations
    struct LEVELDB_EXPORT WriteOptions {
        WriteOptions() = default;

        // If true, the write will be flushed from the operating system
        // buffer cache (by calling WritableFile::Sync()) before the write
        // is considered complete.  If this flag is true, writes will be
        // slower.
        //
        // If this flag is false, and the machine crashes, some recent
        // writes may be lost.  Note that if it is just the process that
        // crashes (i.e., the machine does not reboot), no writes will be
        // lost even if sync==false.
        //
        // In other words, a DB write with sync==false has similar
        // crash semantics as the "write()" system call.  A DB write
        // with sync==true has similar crash semantics to a "write()"
        // system call followed by "fsync()".
        bool sync = false;
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
