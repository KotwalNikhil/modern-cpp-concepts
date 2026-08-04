// Implement your own thread pool

#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
using namespace std;

class ThreadPool
{
    condition_variable cv;
    vector<std::thread>workers;
    mutex mtx;
    std::queue<std::function<void()>> tasks;
    bool stop{};

    void run()
    {
        while(true)
        {
            unique_lock<mutex>lock(mtx);
            cv.wait(lock, [this]{return stop || !tasks.empty();});
            if(stop and tasks.empty())return; // exit while loop is stop is true mostly it is true on destructor other wise you worker thread will keep waiting forever
            auto task = move(tasks.front()); // avoid copy of std::function
            tasks.pop();
            lock.unlock();
            if(task)task();
        }
    }

public:
    ThreadPool(size_t size):workers(size, &ThreadPool::run, this){}
    // since std::thread, cv, mutex are non-copyable done allow pool to be copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // mtx and cv are non-movable too

};