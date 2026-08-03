// practice how to create custom type traits and concepts and use them as SFINAE
// See notes on notebook for more about SFINAE constrainst placements positions

#include <iostream>
#include <type_traits>
#include <concepts>
using namespace std;

class Monster
{
    int id{};
    string name{"moni"};

public:
    void print()
    {
        cout<<name;
    }
};

class Monster_dup
{
    int id{};
    string name{};
};

//create a new type trait
template<typename T, typename = void>
struct has_print : std::false_type{};

template<typename T>
struct has_print<T, std::void_t<decltype(std::declval<T>().print())>> : true_type{};

// create concepts
template<typename T>
concept has_print_concept = requires(T t){
    t.print();
};


/*
To restrict a class with a template constraints at compile time we can use different ways
1. using traits i.e. SFINAE
template< typename T, typename = std::enable_if_t<has_print<T>::value>>

2. using concepts
template<typename T> requires has_print_concept<T> class myClass {...}
or
template<has_print_concept T> class myClass
*/
template<typename T, typename = std::enable_if_t<has_print<T>::value>>
class myClass
{
public:
    void call(T obj)
    {
        obj.print();
    }
};


int main()
{

    myClass<Monster>obj;
    static_assert(has_print_concept<Monster>);
    static_assert(!has_print_concept<Monster_dup>);
    //myClass<Monster_dup> obj2;
    
}
