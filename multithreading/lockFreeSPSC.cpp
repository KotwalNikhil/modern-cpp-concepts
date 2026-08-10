// lock free SPSC model
#include <atomic>
#include <iostream>
#include <queue>
#include <thread>
#include <cassert>
#include <array>

using namespace std;

template<typename T, size_t N>
class SPSC
{
    static_assert((N & (N-1)) == 0, "size of queue should be power of 2");
public:
    // SPSC(size_t capacity):capacity_(capacity), buffer_(capacity){
    //     assert(capacity_ > 0 && (capacity_ & (capacity_ - 1)) == 0 && "capacity must be a power of 2 and > 0");
    // }

    bool push(const T& item)
    {
        size_t head = head_.load(memory_order_relaxed);
        int next = increment(head);
        if(next == tail_.load(memory_order_acquire))
        {
            cout<<"Buffer is full\n";
            return false;
        }

        buffer_[head] = item;
        head_.store(next, memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        size_t tail = tail_.load(memory_order_relaxed);
        if(tail == head_.load(memory_order_acquire))
        {
            cout<<"buffer is empty\n";
            return false;
        }

        item = buffer_[tail];
        tail_.store(increment(tail), memory_order_release);
        return true;
    }

    size_t increment(size_t idx)
    {
        return (idx+1)&(N-1); // make sure (capacity & (capacity-1)) == 0 i.e power of 2
    }

private:
    array<T, N> buffer_;
    alignas(64) atomic<size_t> head_{0}; // To avoid false sharing by preventing head and tail to be present on the same cache line
    alignas(64) atomic<size_t> tail_{0};
};

SPSC<int, 32>spsc;

void producer()
{
    int i=0;
    while(true)
    {
        if(spsc.push(i))
        {
            cout<<"Produced value = "<<i<<endl;
            ++i;
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void consumer()
{
    int value;
    while(true)
    {
        if(spsc.pop(value))
        {
            cout<<"Received value of "<<value<<endl;
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
    }
}

int main()
{
 thread prod(producer);
 thread con(consumer);

 con.join();
 prod.join();

}