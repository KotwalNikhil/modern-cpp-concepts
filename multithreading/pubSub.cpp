#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
using namespace std;

std::mutex mtx;
condition_variable cv;
bool data_ready = false;
int result = 0;


// publisher code
auto publish = [](){
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::unique_lock<std::mutex> lock(mtx);

		result = 50;
		data_ready = true;
		cout << "Data Produced! " <<result<< endl;
		lock.unlock();
		cv.notify_one();
	};

// subscriber code
auto subscribe = [](){

	// locking
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, []{return data_ready;});
    cout<<"data consumed "<<result<<endl;
};

int main() {
	
	std::thread subscriber(subscribe);
	std::thread publisher(publish);
	
	subscriber.join();
	publisher.join();
	
	return 0;
}
