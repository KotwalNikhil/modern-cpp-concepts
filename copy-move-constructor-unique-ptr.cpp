#include<iostream>

template<typename T>
class Smart {
private:
	T* ptr_ {};

public:
	Smart(T* a = nullptr): ptr_(a) {
	}

	~Smart() {
		delete ptr_;
	}


	// copy constructor
	Smart(const Smart& a) {

		ptr_ = new T;
		*ptr_ = *a.ptr_;
	}

	// copy assignment operator
	Smart& operator=(const Smart& a) {
		// self eval
		if (&a == this) {
			return *this;
		}

		delete ptr_;
		ptr_ = new T;
		*ptr_ = *a.ptr_;

		return *this;
	}

	// move const
	Smart(Smart&& a) noexcept 
	:ptr_(a.ptr) 
	{
		a.ptr_ = nullptr;
	}

	// move assignment operator
	Smart& operator=(Smart&& a) noexcept {
		if (&a == this) {
			return *this;
		}

		delete ptr_;

		ptr_ = a.ptr_;
		a.ptr_ = nullptr;

		return *this;
	}

	T& operator*() const {return *ptr_;}
	T* operator->() const {return ptr_;}

	bool isNull() {return ptr_ == nullptr;}

};

class Resource
{
public:
	Resource() { std::cout << "Resource acquired\n"; }
	~Resource() { std::cout << "Resource destroyed\n"; }
};



int main()
{

	#ifndef ONLINE_JUDGE
	freopen("input.txt","r",stdin); //file input.txt is opened in reading mode i.e "r"
	freopen("output.txt","w",stdout);  //file output.txt is opened in writing mode i.e "w"
	#endif

	Smart<Resource> sp = new Resource;
	std::cout<<sp.isNull()<<std::endl;

}
