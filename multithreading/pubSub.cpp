#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
using namespace std;

class PubSub
{

std::mutex mtx_;
condition_variable cv_;
bool running_ = true;
queue<int>buffer_;
size_t capacity_;

public:
	PubSub(size_t capacity) : capacity_(capacity) {}

	void publisher(int val)
	{
		for(int i =0;i<val;i++)
		{
			unique_lock<mutex>lock(mtx_);
			cv_.wait(lock, [this](){return buffer_.size() < capacity_;});
			buffer_.push(i+1);
			cout<<"Publisher published val = "<< (i+1) <<endl;
			cv_.notify_one();
		}

		unique_lock<mutex>lock(mtx_);
		running_ = false;
		cv_.notify_all();
		
	}

	void subscriber()
	{
		while(true)
		{
			unique_lock<mutex>lock(mtx_);
			cv_.wait(lock, [this](){
				return ((buffer_.size() > 0) || (!running_));
			});

			if(buffer_.empty() && running_ == false){
				std::cout << "Subscriber: Publisher finished and buffer empty. Exiting." << std::endl;
				break;
			}

			int val = buffer_.front();
			buffer_.pop();

			cout<<"reading val = " << val <<endl;

			cv_.notify_one();
		}
	}

};

int main() 
{
	PubSub pubsub(50);

	thread t1(&PubSub::publisher, &pubsub, 100);
	thread t2(&PubSub::subscriber, &pubsub);

	t1.join();
	t2.join();
	
	return 0;
}
