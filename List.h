#include <cassert>
#include <iostream>
#include <memory>
#include <initializer_list>

template<class T>
// requires Ord<T>
class List
{
    struct Item
    {
        Item(T v, std::shared_ptr<const Item> const & tail) : _val(v), _next(tail) {}
        T _val;
        std::shared_ptr<const Item> _next;
    };
    friend Item;
    explicit List(std::shared_ptr<const Item> const & items) : _head(items) {}
public:
    // Empty list
    List() {}
    // Cons
    List(T v, List const & tail) : _head(std::make_shared<Item>(v, tail._head))
    {
        assert(tail.isEmpty() || v <= tail.front());
    }
    bool isEmpty() const { return !_head; } // conversion to bool
    T front() const
    {
        assert(!isEmpty());
        return _head->_val;
    }
    List popped_front() const
    {
        assert(!isEmpty());
        return List(_head->_next);
    }
    // Additional utilities
    List inserted(T v) const
    {
        if (isEmpty() || v <= front())
            return List(v, List(_head));
        else {
            return List<T>(front(), popped_front().inserted(v));
        }
    }
    // For debugging
    int headCount() const { return _head.use_count(); }
private:
    std::shared_ptr<const Item> _head;
};


template<class T>
List<T> merged(List<T> const & a, List<T> const & b)
{
    if (a.isEmpty())
        return b;
    if (b.isEmpty())
        return a;
    if (a.front() <= b.front())
        return List<T>(a.front(), merged(a.popped_front(), b));
    else
        return List<T>(b.front(), merged(a, b.popped_front()));
}

// For debugging
template<class T>
void print(List<T> lst)
{
    if (lst.isEmpty()) {
        std::cout << std::endl;
    }
    else {
        std::cout << "(" << lst.front() << ", " << lst.headCount() - 1 << ") ";
        print(lst.popped_front());
    }
}
