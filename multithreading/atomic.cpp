#include<thread>
#include<atomic>
#include<iostream>
using namespace std;

class Spin {
public:

    Spin(): flag_(ATOMIC_FLAG_INIT){}

    void lock() {
        while(flag_.test_and_set()) {}
    }

    void unlock() {
        flag_.clear();
    }

private:
    std::atomic_flag flag_;
};



void workOnResource(Spin& spin, int& x) {
    spin.lock();

    // shared crtical section
    // book movie ticket
    std::cout<<"before x= "<<x<<endl;
    x++;
    std::cout<<"after x= "<<x<<endl;

    spin.unlock();
}

int main() {
    Spin spin;
    int x = 0;

    thread t(workOnResource,spin, x);
    thread t2(workOnResource,spin, x);

    t.join();
    t2.join();
}