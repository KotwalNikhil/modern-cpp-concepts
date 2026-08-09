// implement your own future promise in C++
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <exception>

using namespace std;

template <typename T>
struct SharedState
{
    T value;
    std::exception_ptr exception;
    bool ready{false};
    mutex mtx;
    condition_variable cv;
    bool futureRetrived{false};
};

template <typename T>
class Future
{
    std::shared_ptr<SharedState<T>> state;
    bool retrived{false};
public:
    Future(std::shared_ptr<SharedState<T>> state):state(state){}
    // not returing T& as future/promise obj reference can become dangling
    T get()
    {
        if(retrived) throw std::runtime_error("future already retrived");
        unique_lock<mutex>lock(state->mtx);
        state->cv.wait(lock, [this]{return state->ready;});
        retrived = true;

        if(state->exception){ rethrow_exception(state->exception); }
        return state->value;
    }
};

template <typename T>
class Promise
{
    std::shared_ptr<SharedState<T>> state;
public:
    Promise():state(make_shared<SharedState<T>>()){}
    void set_value(const T& val)
    {
        lock_guard<mutex>lock(state->mtx);
        if(state->ready) throw runtime_error("Promise already satisfied");
        state->value = val;
        state->ready = true;
        state->cv.notify_all(); // prefer to keep it outside lock
    }

    Future<T> get_future()
    {
        lock_guard<mutex>lock(state->mtx);
        if(state->futureRetrived) throw std::runtime_error("future already retrived");
        state->futureRetrived = true;
        return Future<T>(state);
    }

    void set_exception(std::exception_ptr ex)
    {
        lock_guard<mutex>lock(state->mtx);
        if(state->ready) throw runtime_error("Promise already satisfied");
        state->exception = ex;
        state->ready = true;
        state->cv.notify_all(); // prefer to keep it outside lock
    }

    // what happens when promise is not fullfilled that is promise exists before calling set_value or exception
    // f.get() will wait forever in that case - Destructor should handle that
    ~Promise()
    {
        lock_guard<mutex>lock(state->mtx);
        if(!state->ready)
        {
            // cant call set_exception as it has lock mutex so it will be deadlock
            state->exception =
                std::make_exception_ptr(
                    std::runtime_error("broken promise"));
            state->cv.notify_all();
        }
    }
};

void oddSum(Promise<int>prom, int s, int e)
{
    std::cout<<this_thread::get_id()<<" calculating sum...\n";
    long long ans = 0;
    for(int i=s;i<e;i++)
    {
        if(i%2==1)ans+=i;
    }
    prom.set_value(ans);
}

int main()
{
    Promise<int>prom;
    Future<int> f= prom.get_future();
    std::thread t(oddSum, move(prom), 2, 10000);
    cout<<this_thread::get_id()<<" after the function\n";
    cout<<this_thread::get_id()<<" ="<<f.get();
    cout<<this_thread::get_id()<<" ="<<f.get();
    t.join();
    return 0;
}