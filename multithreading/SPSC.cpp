// Implement non-blocking SPSC queue lock based

#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
using namespace std;

template <typename T>
class SPSC
{
    vector<T>v;
    size_t head;
    size_t tail;
    std::mutex mtx;
    std::condition_variable cond;

public:
    SPSC(size_t cap):v(cap), head(0), tail(0){}

    bool push(const T& val)
    {
        lock_guard<mutex>lock(mtx);
        size_t next = (head + 1) % v.size();
        // push() is currently non-blocking. A true blocking push() would use cond.wait(...) while the queue is full and wake when a consumer calls pop().
        if(next == tail){cout<<"queue is full\n"; return false;}

        v[next] = val;
        head = next;
        cond.notify_one();
        
        return true;
    }

    bool pop(T& out)
    {
        unique_lock<mutex> lock(mtx);
        cond.wait(lock, [&](){return (tail != head);});
        size_t next = (tail+1) % v.size();
        out = v[next];
        tail = next;
        return true;
    }
    
};

SPSC<int>spsc(10);
void producer()
    {
        static int val = 0;
        while(true)
        {
            if(spsc.push(val))
            {
                cout<<this_thread::get_id()<<" procducer outputs = "<<val<<"\n";
                val++;
            }
            this_thread::sleep_for(chrono::milliseconds(400));
        }
    }

    void consumer()
    {
        int out;
        while(true)
        {
            if(spsc.pop(out))
            {
                cout<<this_thread::get_id()<<" Consumer received = "<<out<<"\n";
            }
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }

int main()
{
    cout<<"hello worlds\n";
    thread prod(producer);
    thread con(consumer);

    prod.join();
    con.join();
}

/*
Follow up- 
can you extend this into SPMC ?
since we use the same mtx in pop so just creating a new thread for consumer will be enough
only one consumer will pop at a time
Also its fine to notify_one instead of notify_all in prodcuer as 
Only one gets the mutex, the others immediately go back to sleep. This is called the thundering herd problem and wastes CPU cycles.
*/