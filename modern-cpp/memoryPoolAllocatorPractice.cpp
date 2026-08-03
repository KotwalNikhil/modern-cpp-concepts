// Implement your own memory pool allocation

#include <iostream>
#include <vector>
#include <memory>
using namespace std;

/*
few imp things in this code
1. use of ::operator new (size)
2 use blockSize =  max(sizeof Node, sizeof T) see explanation below
3. create list of chunks using Node*
4. deallocate block and attach it as the head
5. reinterpret_cast to char* pointer for correct pointer arthiematic
*/
template <typename T>
class MemoryPool
{
public:
    MemoryPool(size_t chunks):poolSize(chunks)
    {
        /*
        * since sizeof Node is 8 bytes it can happen that sizeof T is less then that maybe 1 byte in that case we will allocate only 1000 bytes if chunks=1000
        * then our list of chunks wont work hence we need to take the max
        */
        size_t chunkSize = std::max(sizeof(Node), sizeof(T)); 
        storage = ::operator new (chunks * chunkSize);
        head = reinterpret_cast<Node*>(storage);
        Node* temp = head;
        for(int i=0;i<chunks-1;++i)
        {
            temp->next = reinterpret_cast<Node*>(reinterpret_cast<char*>(temp) + chunkSize);
            temp=temp->next;
        }
        temp->next = nullptr;
    }

    ~MemoryPool()
    {
        ::operator delete(storage);
    }

    T* allocate()
    {
        if(!head)return nullptr; // pool exhausted
        Node* node = head;
        head=head->next;
        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* ptr)
    {
        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = head;
        head = node;
    }

private: 
    // To build a free list of available memory blocks
    struct Node
    {
        Node* next;
    };

    size_t poolSize;
    void* storage;
    Node* head;
};

struct Order
{
    /* data */
    int orderId;
    double price;
    int quantity;
    Order() : orderId(0), price(0.0), quantity(0) {}
    Order(int id, double p, int q) : orderId(id), price(p), quantity(q) {}

    void display() const
    {
        std::cout << "Order#" << orderId << ": " << quantity << " @ $" << price << '\n';
    }
};


int main()
{
    MemoryPool<Order> pool(1000);
    std::vector<Order*> orders;
    
    // Allocate 100 orders (not 1000 since it's just a demo)
    for(int i = 0; i < 100; ++i)
    {
        Order* order = pool.allocate();
        if(order)
        {
            new (order) Order(i + 100, 50.0 + i * 0.1, 10 + i);
            orders.push_back(order);
        }
    }

    for(auto order : orders)
    {
        order->display();
        order->~Order(); // nicee
        pool.deallocate(order);
    }

    orders.clear();
}