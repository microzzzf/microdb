#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <iostream>
#include<atomic>

#define DEBUG 1

namespace microdb
{

template<typename Key>
class SkipList
{
public:
    static const unsigned int MAXLEVEL = 4;

    class Node
    {
    public:
        explicit Node();
        bool setNext(Node* expected, Node* node, unsigned int level);
        void setNextWithoutAtomic(Node* node, unsigned int level);
        Node* getNext(unsigned int level);
        Key& key();
        void setKey(const Key& key);
    private:
        Key key_;
        std::atomic<Node*> next_[MAXLEVEL]; 
    };

    class Iterator
    {
    public:
        Iterator();
        explicit Iterator(Node* node);  
        Iterator next(unsigned int level);
        Node* operator->();
        Node& operator*();
        bool operator==(Iterator& itr) const;
        bool operator!=(Iterator& itr) const;
        Key& key();
        Node* node() const;
    private:
        Node* node_;
    };
    
    explicit SkipList();
    void put(Key& key);
    bool get(const Key& key);
    void remove(const Key& key);
    void print();
    Iterator& begin();
    Iterator& end();

#if DEBUG
    struct _debug_counter
    {
        std::atomic<unsigned int> inserted_counter;
        std::atomic<unsigned int> not_inserted_counter;
        std::atomic<unsigned int> in_list_counter;
    }debug_counter;

    void inListCount();
#endif
private:
    int random();
    SkipList<Key>::Iterator lowerBound(const Key& key, unsigned int level);
    SkipList<Key>::Iterator upperBound(Key& key, unsigned int level);

    Node* head_;
    Iterator begin_;
    Iterator end_;
};

template<typename Key>
SkipList<Key>::Node::Node()
{
    Key key;
    key_ = key;
    for (auto i = 0; i < MAXLEVEL; ++i)
    {
        next_[i] = nullptr;
    }
}

template<typename Key>
bool SkipList<Key>::Node::setNext(Node* expected, Node* node, unsigned int level)
{
    return next_[level].compare_exchange_strong(expected, node, std::memory_order_relaxed, std::memory_order_relaxed);
}

template<typename Key>
void SkipList<Key>::Node::setNextWithoutAtomic(Node* node, unsigned int level)
{
        next_[level].store(node);
}

template<typename Key>
typename SkipList<Key>::Node* SkipList<Key>::Node::getNext(unsigned int level)
{
    return next_[level].load(std::memory_order_relaxed);
}

template<typename Key>
Key& SkipList<Key>::Node::key()
{
    return key_;
}

template<typename Key>
void SkipList<Key>::Node::setKey(const Key& key)
{
    key_ = key;
}

template<typename Key>
SkipList<Key>::Iterator::Iterator(Node* node)
{
    node_ = node; 
}

template<typename Key>
SkipList<Key>::Iterator::Iterator()
{
        node_ = nullptr;
}

template<typename Key>
typename SkipList<Key>::Iterator SkipList<Key>::Iterator::next(unsigned int level)
{
    Iterator tmp(node_);
    if (tmp.node_)
    {
        tmp.node_ = tmp.node_->getNext(level);
    }
    return tmp;
}

template<typename Key>
typename SkipList<Key>::Node* SkipList<Key>::Iterator::operator->()
{
    return node_;
}

template<typename Key>
typename SkipList<Key>::Node& SkipList<Key>::Iterator::operator*()
{
    return *node_;
}

template<typename Key>
bool SkipList<Key>::Iterator::operator==(Iterator& itr) const
{
    return (node_ == itr.node_);
}

template<typename Key>
bool SkipList<Key>::Iterator::operator!=(Iterator& itr) const
{
    return (node_ != itr.node_);
}

template<typename Key>
Key& SkipList<Key>::Iterator::key()
{
    return node_->key();
}

template<typename Key>
typename SkipList<Key>::Node* SkipList<Key>::Iterator::node() const
{
    return node_;
}

template<typename Key>
typename SkipList<Key>::Iterator& SkipList<Key>::begin()
{
    return begin_;
}

template<typename Key>
typename SkipList<Key>::Iterator& SkipList<Key>::end()
{
    return end_;
}

#if DEBUG
template<typename Key>
void SkipList<Key>::inListCount()
{
    for (auto itr = begin().next(0); itr != end(); itr = itr.next(0))
    {
        ++debug_counter.in_list_counter;
    }   
}
#endif

template<typename Key>
int SkipList<Key>::random()
{
    return rand() % MAXLEVEL;
}

template<typename Key>
SkipList<Key>::SkipList()
{
    head_ = new Node();
    begin_ = Iterator(head_);
    end_ = Iterator(nullptr);
#if DEBUG
    debug_counter.inserted_counter = 0;
    debug_counter.not_inserted_counter = 0;
    debug_counter.in_list_counter = 0;
#endif
}

template<typename Key>
typename SkipList<Key>::Iterator SkipList<Key>::lowerBound(const Key& key, unsigned int level)
{
    auto itr = begin();
    for (itr = begin().next(level); itr != end() && key > itr->key(); itr = itr.next(level))
    {
        
    }
    return itr;
}

template<typename Key>
typename SkipList<Key>::Iterator SkipList<Key>::upperBound(Key& key, unsigned int level)
{
    auto last_itr = begin();
    for (auto itr = begin().next(level); itr != end() && key > itr->key(); itr = itr.next(level))
    {
        last_itr = itr;
    }
    return last_itr;
}

template<typename Key>
void SkipList<Key>::put(Key& key)
{
    int level = random();
    SkipList<Key>::Iterator pre;
    SkipList<Key>::Iterator next;
    Node* new_node = new Node();
    new_node->setKey(key);

    for (auto i = level; i >= 0; --i)
    {
        while (true)
        {
            pre = upperBound(key, i);
            next = pre.next(i);
            if (next != end())
            {
                if (next->key() == key)
                {
    #if DEBUG
                    ++debug_counter.not_inserted_counter;
    #endif
                    //std::cout<<"The key has existed! "<<key<<" Just return"<<std::endl;
                    delete new_node;
                    return;
                }
            }
            new_node->setNextWithoutAtomic(next.node(), i);
            auto tmp = next.node();
            if (pre->setNext(tmp, new_node, i))
            {
                break;
            }
        }
    }
#if DEBUG
    ++debug_counter.inserted_counter;
#endif
}

template<typename Key>
bool SkipList<Key>::get(const Key& key)
{

}

template<typename Key>
void SkipList<Key>::print()
{
    std::cout<<"################## Print my Skiplist ##################"<<std::endl;
    for (auto i = 0; i < MAXLEVEL; ++i)
    {
        std::cout<<"Level "<<i<<" :"<<std::endl;
        for (auto itr = begin().next(i); itr != end(); itr = itr.next(i))
        {
            std::cout<<itr->key()<<" "; 
        }
        std::cout<<std::endl;
    }
    std::cout<<"########## Print my Skiplist, it is the end ##########"<<std::endl;
}

} // namespace microdb

#endif // _SKIPLIST_H
