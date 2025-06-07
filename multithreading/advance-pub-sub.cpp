#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unordered_map>
#include<string>
#include<queue>

using namespace std;

class Subscriber;

class Message{

	string message_;
public:

	Message(string msg):message_(msg) {}
	string getMessage() {
		return message_;
	}
};

class Broker
{

	unordered_map<string, vector<Subscriber*>> subscribers_;
	unordered_map<string, std::queue<Message*>> topics_;
	mutex mtx;
	condition_variable cv;

public:
	void publish(const string& topic, Message* message);
	void subscribe(const string& topic, Subscriber* sub);
	void processMesaage();
};

class Publisher
{
	Broker* broker_;

public:
	Publisher(Broker& broker): broker_(&broker){}
	void publish(string topic, Message* message) {
		broker_->publish(topic, message);
	}
};

class Subscriber
{
	Broker* broker_;
public:
	Subscriber(Broker& broker):broker_(&broker){}
	void subscribe(const string& topic) {
		broker_->subscribe(topic, this);
	}
	void onAlphaEvent(Message* msg, const string& topic) {
		cout<<"Received msg ="<<msg->getMessage()<<" on topic "<<topic<<endl;
	}
};

void Broker::subscribe(const string &topic, Subscriber *sub) {
	unique_lock<mutex> lock(mtx);
    subscribers_[topic].push_back(sub);
}

void Broker::publish(const string &topic, Message *message) {
	unique_lock<mutex> lock(mtx);
    topics_[topic].push(message);
    cv.notify_all();
    // for (auto &sub : subscribers_[topic]) {
    //     sub->onAlphaEvent(message, topic);
    // }
}

void Broker::processMesaage() {
	unique_lock<mutex> lock(mtx);
	cv.wait(lock, [this](){
		for(auto topic: topics_) {
			if( !topic.second.empty())return true;
		}
		return false;
	});

	for(auto topic: topics_) {
		while(not topic.second.empty()) { // check all messages
			Message* msg = topic.second.front();
			topic.second.pop();

			// send this msg to all its subscribers
			for(auto sub: subscribers_[topic.first]) {
				sub->onAlphaEvent(msg, topic.first);
			}
		}
		
	}
}


int main() {
	
	Broker broker;
	Publisher pub(broker);
	Subscriber s1(broker), s2(broker);

	s1.subscribe("A");
	s2.subscribe("A");
	s2.subscribe("B");

	std::thread t(&Broker::processMesaage, &broker);

	pub.publish("A", new Message("this msg is for topic A"));
	pub.publish("B", new Message("this msg is for topic B"));

	t.join();
	return 0;
}



/*
Area of improvement in the above code
Message is created in heap but not deleted, use shared ptr to send messages

Single-Pass Processing in processMessage:
processMessage runs in a separate thread (t1), waits for any message, processes all currently available messages, and then the thread immediately join()s in main.
This means your PubSub system only processes messages once. If p1.publish were called after t1.join() or if there were a delay, no more messages would be processed.
Fix: processMessage should typically run in a continuous loop, constantly waiting for new messages, processing them, and then waiting again. You'll need a mechanism to signal this thread to shut down gracefully.

PubSub stores raw Subscriber* pointers in subscribers_. If a Subscriber object is destroyed (e.g., s1 or s2 go out of scope in main), the PubSub instance will hold a dangling pointer. Accessing this pointer later would result in undefined behavior.
Fix: Use std::shared_ptr<Subscriber> or std::weak_ptr<Subscriber> for subscriber management. std::weak_ptr is ideal to avoid circular references if subscribers also hold shared_ptr to PubSub.

Specific onAlphaEvent:
The Subscriber class has a hardcoded onAlphaEvent method. In a real-world pub-sub system, subscribers might want to register different callback functions for different topics or have different processing logic.
Improvement: Use std::function to allow subscribers to register a generic callable (lambda, function pointer, or functor) as their message handler.

The above code also support multiple Publishers:

The PubSub::publish method uses a std::mutex to protect access to the topics_ map and the message queue. This means multiple Publisher instances (or multiple threads calling publish from different publishers) can call publish concurrently without causing data corruption. The mutex ensures that only one publisher modifies the shared topics_ map at any given time.
In the main function of the improved code, I've added Publisher p2(pubsub); and p2.publish(...) calls to explicitly demonstrate this.
*/
