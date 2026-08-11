// Implement your own thread pool

#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
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
    ThreadPool(size_t size)
    {
        for(int i=0;i<size;i++)
        {
            workers.emplace_back([this]{
                run();
            });
        }
    }

    // using template as it makes submit works for any lambds, fun ptr, functions and emplace_back will cast them into function<void()> inside tasks
    // template<typename F>
    // void submit(F&& fun) // F&& is a forwarding reference, can be used to pass both lvalue, rvalue
    // {
    //     {
    //         lock_guard<mutex> lock(mtx);
    //         tasks.emplace(std::forward<F>(fun)); // preserves the value of expression 
    //     }
    //     cv.notify_one();
    // }

    // what if we need a return from the thread process- we use packaged_Task in a lambda
    template <typename F, typename ...Args>
    auto submit(F&& f, Args&&... args) -> std::future<invoke_result_t<F, Args...>>
    {
        using ReturnType = invoke_result_t<F, Args...>;
        auto task = std::make_shared<packaged_task<ReturnType()>>(
            [f = std::forward<F>(f),
            ...args = forward<Args>(args)]() mutable{
                return f(args...);
            }
        );

        auto result = task->get_future();
        {
            lock_guard<mutex>lock(mtx);
            tasks.emplace([task]{
            (*task)();
                });
        }
        cv.notify_one();
        return result;
    }

    ~ThreadPool()
    {
        {
            lock_guard<mutex>lock(mtx);
            stop = true;
        }

        cv.notify_all();
        for(auto& it: workers) // cant do const auto as we cannot call join from a const object
        {
            if(it.joinable())it.join();
        }
    }

    // since std::thread, cv, mutex are non-copyable done allow pool to be copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // mtx and cv are non-movable too
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
};

void print()
{
    cout<<std::this_thread::get_id()<< " I am priting something\n";
}

int main()
{
    //cout<<std::this_thread::get_id()<< " Hello world\n";
    ThreadPool pool(2);
    pool.submit([]{cout<<std::this_thread::get_id()<< " I am priting something\n";});
    pool.submit([]{cout<<std::this_thread::get_id()<< " I am priting something\n";});
    pool.submit([]{cout<<std::this_thread::get_id()<< " I am priting something\n";});
    auto fut = pool.submit([]{cout<<std::this_thread::get_id()<< " I am calculating 43 something\n"; return 49;});
    int x = fut.get();
    cout<<"hello world = "<< x;
}