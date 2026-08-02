#include <bits/stdc++.h>
using namespace std;

// topic based pubSub model

using Feed = std::string;
using Callback = std::function<void(int)>;

class PubSub
{
public:
	struct Subscription
	{
		int id;
		Feed feed;
	};

	Subscription subscribe(Feed feed, Callback cb)
	{
		counter++;
		mp[feed].push_back({counter, std::move(cb)});
		return {counter, feed};
	}

	void unSubscribe(Subscription& sub)
	{
		auto it = mp.find(sub.feed);
		if(it == mp.end())return;

		// remove_if moves the elements in the front and return the new logical end
		// v.erase(new_end, end)
		auto& v = it->second;
		v.erase(remove_if(v.begin(), v.end(), [&](const auto& p){
			return p.first == sub.id;
		}), v.end());

		if(v.empty())
		{
			mp.erase(it);
		}
	}

	void publish(Feed feed, int data)
	{
		auto it = mp.find(feed);
		if(it == mp.end())return;

		auto& v = it->second;

		for(auto& cb : v)
		{
			cb.second(data);
		}
	}

private:
	unordered_map<Feed, vector<pair<int,Callback>>>mp;
	int counter{0};
};

int main()
{
	PubSub pubsub;
	auto sub1 = pubsub.subscribe("eurusd.ebs", [](int data){
		cout<<"Subscriber 1ebs received data ="<<data<<endl;
	});

	auto sub2 = pubsub.subscribe("eurusd.ebs", [](int data){
		cout<<"Subscriber 2ebs received data ="<<data<<endl;
	});

	auto sub3 = pubsub.subscribe("eurusd.cme", [](int data){
		cout<<"Subscriber 3cme received data ="<<data<<endl;
	});

	pubsub.publish("eurusd.ebs", 100);
	pubsub.publish("eurusd.ebs", 200);
	pubsub.publish("eurusd.cme", 300);

	pubsub.unSubscribe(sub2);

	pubsub.publish("eurusd.ebs", 100);
	pubsub.publish("eurusd.ebs", 200);
	pubsub.publish("eurusd.cme", 300);


	return 0;

}
