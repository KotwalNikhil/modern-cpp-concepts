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
