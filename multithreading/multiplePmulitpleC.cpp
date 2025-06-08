#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <chrono>

using namespace std;

class PubSub
{
    std::mutex mtx_;
    condition_variable cv_;
    queue<int> buffer_;
    size_t capacity_;
    int producer_count_; // number of active producers

public:
    PubSub(size_t capacity, int producer_count = 1)
        : capacity_(capacity), producer_count_(producer_count) {}

    void publisher(int id, int val)
    {
        for (int i = 0; i < val; i++)
        {
            unique_lock<mutex> lock(mtx_);
            cv_.wait(lock, [this]() {
                return buffer_.size() < capacity_;
                });
            buffer_.push(i + 1 + id * 10000); // unique value per producer for demonstration
            cout << "Publisher " << id << " published val = " << buffer_.back() << endl;
            cv_.notify_all();
        }
        // Producer finished
        {
            unique_lock<mutex> lock(mtx_);
            producer_count_--;
            cout << "Publisher " << id << " finished. Remaining producers: " << producer_count_ << endl;
        }
        cv_.notify_all();
    }

    void subscriber(int id)
    {
        while (true)
        {
            unique_lock<mutex> lock(mtx_);
            cv_.wait(lock, [this]() {
                return !buffer_.empty() || producer_count_ == 0;
                });

            if (buffer_.empty() && producer_count_ == 0)
            {
                cout << "Subscriber " << id << ": No more producers and buffer empty. Exiting." << endl;
                break;
            }

            int val = buffer_.front();
            buffer_.pop();

            cout << "Subscriber " << id << " reading val = " << val << endl;

            cv_.notify_all();
        }
    }
};

int main()
{
    {
        cout << "=== Single Producer / Multiple Consumers ===" << endl;
        PubSub pubsub(10, 1); // capacity 10, 1 producer

        thread producer(&PubSub::publisher, &pubsub, 0, 50);

        vector<thread> consumers;
        for (int i = 0; i < 3; i++)
        {
            consumers.emplace_back(&PubSub::subscriber, &pubsub, i);
        }

        producer.join();
        for (auto &t : consumers)
            t.join();
    }

    cout << endl;

    {
        cout << "=== Multiple Producers / Multiple Consumers ===" << endl;
        int num_producers = 3;
        PubSub pubsub(20, num_producers); // capacity 20, 3 producers

        vector<thread> producers;
        for (int i = 0; i < num_producers; i++)
        {
            producers.emplace_back(&PubSub::publisher, &pubsub, i, 30);
        }

        vector<thread> consumers;
        for (int i = 0; i < 5; i++)
        {
            consumers.emplace_back(&PubSub::subscriber, &pubsub, i);
        }

        for (auto &p : producers)
            p.join();
        for (auto &c : consumers)
            c.join();
    }

    return 0;
}

