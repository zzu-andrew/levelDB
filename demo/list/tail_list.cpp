//
// Created by wangyz38535 on 2024/4/24.
//

#include <iostream>

// 尾插式循环链表
class TailList;

class Snapshot {
public:
    ~Snapshot() = default;
};

// Snapshots are kept in a doubly-linked list in the DB.
// Each SnapshotImpl corresponds to a particular sequence number.
class NodeImpl : public Snapshot {
public:
    explicit NodeImpl(uint64_t sequence_number)
            : sequence_number_(sequence_number) {}

    uint64_t sequence_number() const { return sequence_number_; }

private:
    friend class TailList;

    // NodeImpl is kept in a doubly-linked circular list. The TailList
    // implementation operates on the next/previous fields direcly.
    NodeImpl* prev_{};
    NodeImpl* next_{};

    const uint64_t sequence_number_;
};

class TailList {
public:
    TailList() : head_(0) {
        head_.prev_ = &head_;
        head_.next_ = &head_;
    }

    bool empty() const { return head_.next_ == &head_; }
    NodeImpl* oldest() const {
        return head_.next_;
    }
    NodeImpl* newest() const {
        return head_.prev_;
    }

    // new 出一个node节点，并把对应的node放到链表结尾
    NodeImpl* New(uint64_t sequence_number) {

        auto* snapshot = new NodeImpl(sequence_number);

        snapshot->next_ = &head_;
        snapshot->prev_ = head_.prev_;
        snapshot->prev_->next_ = snapshot;
        snapshot->next_->prev_ = snapshot;
        return snapshot;
    }

    // Removes a NodeImpl from this list.
    //
    // The snapshot must have been created by calling New() on this list.
    //
    // The snapshot pointer should not be const, because its memory is
    // deallocated. However, that would force us to change DB::ReleaseSnapshot(),
    // which is in the API, and currently takes a const Snapshot.
    static void Delete(const NodeImpl* snapshot) {
        snapshot->prev_->next_ = snapshot->next_;
        snapshot->next_->prev_ = snapshot->prev_;
        delete snapshot;
    }

private:
    // Dummy head of doubly-linked list of snapshots
    NodeImpl head_;
};


int main(int argc, char* argv[]) {

    TailList  list;

    list.New(1);
    list.New(2);
    list.New(3);
    list.New(3);
    list.New(3);
    list.New(3);

    do {

        auto lpNode = list.newest();
        std::cout << lpNode->sequence_number() << std::endl;
        TailList::Delete(lpNode);

    } while (!list.empty());


    return 0;
}

