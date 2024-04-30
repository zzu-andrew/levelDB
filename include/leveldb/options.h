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

        // 数据库允许打开文件的最大数量，当你预估数据库存储的内容可能会很大时，可以根据需要适当增大该值
        // 进行预算时，按照每2M内容一个文件来进行预算
        int max_open_files = 1000;

        // 控制blocks，DB会将用户的数据存储成一系列的blocks，一个block是一个从磁盘中加载的单元
        // 如果非空，会使用用户指定的Cache，如果为空，levelDB会创建一个默认的8M的内部Cache
        Cache *block_cache = nullptr;

        // 用来指定每个block用户数据的大小，这些大小都是数据压缩之前的大小
        // 当压缩之后存储在磁盘上的数据可能远小于这个值的大小(如果启用了压缩功能)，该值可以根据需要动态的进行改变
        size_t block_size = 4 * 1024;

        //在 LevelDB 中，block_restart_interval 是用于控制块重启的变量。LevelDB 使用块（block）来组织和存储数据，
        // 每个块是一个数据块，包含多个键值对。block_restart_interval 确定了在块中重新启动（restart）前的连续键值对的数量。
        //在 LevelDB 的块中，每个重新启动点都需要存储键的前缀，这样可以在搜索和查找操作中更快地定位到特定的键。
        // 重新启动点是为了减少在每个键值对中存储完整键的开销。
        //block_restart_interval 变量定义了在一个块中连续键值对的数量。
        // 当达到 block_restart_interval 时，LevelDB 将创建一个新的重新启动点，并存储相应的键前缀。
        // 这样，在进行查找时，LevelDB 可以根据重新启动点的位置快速定位到特定的键。
        //通过调整 block_restart_interval 的值，可以在性能和空间消耗之间进行权衡。
        // 大部分客户可能用不到该值
        int block_restart_interval = 16;

        // leveldb会创建文件用于记录数据，该值制定了每个文件的大小，在每次将要超过该值时，levelDB会创建新的文件
        // 大部分客户端应该保持改制不变，但是当你的系统对大文件更加高效的时候，你应该考虑适当的增加该值的大小
        size_t max_file_size = 2 * 1024 * 1024;

        // 用来指定压缩算法，可以动态修改
        // 默认: kSnappyCompression压缩算法，能给出极速但是轻量级压缩.
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

        // 使用指定的过滤条件，来讲减少对磁盘的访问(设置为NewBloomFilterPolicy之后，能很大程度的减少对磁盘的访问次数)
        // NewBloomFilterPolicy()
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
        // 内部存了一个uint64_t类型的序列号
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
