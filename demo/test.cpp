//
// Created by wangyz38535 on 2021/10/21.
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>

#include <type_traits>
#include <utility>
#include <thread>
#include <set>
#include <iostream>
#include <cassert>
#include <functional>
#include <map>
#include <vector>

#include "leveldb/write_batch.h"
#include "leveldb/db.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include <cstdlib> // 用于Windows上的_mkdir函数
#include <sys/stat.h> // 用于Linux上的mkdir函数

using namespace std;


struct ConsumptionOffset {
    std::map<TopicName, std::map<PatitionNo, Offset>> consumeOffsets;   // 单个消费者组偏移量信息
};

struct ConsumerInfo {
    std::map<TopicName, std::vector<partitionNo>>  consumerTopics; // 消费主题信息
    uint64_t    arbVersion; //< 消费者仲裁版本号
}

struct ConsumerCoordinatorInfo {
    std::map<ConsumerInstanceId, ConsumerInfo> consumerCoordinatorResults; //< 消费协调结果
};

class ConsumerCoordinator {
public:
    ConsumerCoordinator(std::string &groupId, ConsumerCoordinatorInfo &consumerCoordinatorInfo) {}
    // 新增消费者
    int32_t AddConsumer();
    // 减少消费者
    int32_t DeleteConsumer();
    // 网络中断
    int32_t NetworkDisconnected();



private:
    std::string m_groupId; //< 消费者组 uuid
    ConsumerCoordinatorInfo m_consumerInfo; //<  消费者实例id <==> 消费者实例消费分配方案

};


class ConsumerCoordinatorOffsetMng {
public:
    uint64_t GetOffset(std::string& groupId, std::string& topicName, int32_t partitionNo) { }

private:
    std::map<GroupId, ConsumptionOffset>  m_consumerrCoordinatorOffset; //< GroupId <==> ConsumerCoordinatorOffset
};



int main() {


    return 0;
}