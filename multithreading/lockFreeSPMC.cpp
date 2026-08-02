// Implement your own lock free SPMC queue

/******************************************************************************

There are two ways - one with using a full flag and using all its capacity
2 using only head, tail and sacrifcing 1 slot to maintain consistency

[DESIGN A — with full_ flag]
//   • Uses all N slots
//   • Needs 3 atomics: head_, tail_, full_
//   • full_ is a SEPARATE atomic — can never be read atomically WITH head_/tail_
//   • This inconsistency window makes it fragile under contention
//   • empty:  tail == head AND full_ == false
//   • full:   tail == head AND full_ == true   ← same index condition, different flag
//
// [DESIGN B — capacity-1 trick, below]
//   • Allocates N+1 slots internally, exposes N usable slots to caller
//   • Needs only 2 atomics: head_, tail_
//   • empty/full derived purely from head and tail — no flag, no inconsistency
//   • empty: tail == head
//   • full:  (head + 1) % capacity_ == tail    ← unambiguous, no flag needed
//   • One slot is always "wasted" (the gap slot between head and tail when full)
//   • Trade: lose 1 slot, gain correctness + one fewer atomic + one fewer cache line bounce
*******************************************************************************/
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <condition_variable>

using namespace std;

template<typename T>
struct SPMC
{
    SPMC(size_t cap): capacity_(cap), v_(cap){}

    bool push(T val);
    bool pop(T& out);
    size_t increment(size_t idx);
private:

    vector<T>v_;
    size_t capacity_;
    alignas(64) atomic<size_t>head_{0};
    alignas(64) atomic<size_t>tail_{0};
};

template<typename T>
inline bool SPMC<T>::push(T val)
{
    size_t head = head_.load(memory_order_relaxed);
    size_t tail = tail_.load(memory_order_acquire);

    size_t next = (head + 1 ) % capacity_;

    if(next == tail)return false;
    v_[head] = val;
    
    head_.store(next, memory_order_release);
    return true;
}

template<typename T>
inline bool SPMC<T>::pop(T& out)
{
    while( true) // cover it with loop as we use CAS, note this is something which we didn't do it in SPSC
    {
        size_t tail = tail_.load(std::memory_order_acquire); // since tail can be accessed by multiple threads, we use acquire
        size_t head = head_.load(std::memory_order_acquire);

        if (tail == head) // empty condition
            return false;

        size_t next = (tail + 1) % capacity_;
        out = v_[tail];
        // because there could be multiple threads doing increement on tail we want consistency hence using CAS( conditional atmoic swap)
        if (tail_.compare_exchange_weak(tail, next, memory_order_acq_rel, memory_order_acquire))
        {
            return true;
        }
    }  
}

SPMC<int>spmc{10};

void producer(int val)
{
    int i=0;
    while(true)
    {
        while(i<val && spmc.push(i))
        {
            cout<<this_thread::get_id()<<" published value= "<<i<<endl;
            this_thread::sleep_for(chrono::milliseconds(500));
            i++;
        }
    }
    
}

void consumer()
{
    while(true)
    {
        int val;
        if(spmc.pop(val)){
            cout<<this_thread::get_id()<<" Consumer value= "<<val<<endl;
            this_thread::sleep_for(chrono::milliseconds(700));

            }
    }
}

int main()
{
    thread prod(producer, 20);
    thread con1(consumer);
    thread con2(consumer);

    prod.join();
    con1.join();
    con2.join();

    return 0;
}