#include <parallel.hpp>
#include <iostream>
#include <vector>
#include <thread>

std::mutex syncobj;

void suspend_func(int i)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    {
        std::lock_guard<std::mutex> lg(syncobj);
        std::cout << "thread id = " << std::this_thread::get_id() << " iteration = " << i << std::endl;
    }
}

class array_wrapper
{
    std::vector<size_t> &_arr;

public:
    array_wrapper(std::vector<size_t> &arr) : _arr{arr} {}
    void operator()(size_t i) const
    {
        if (i < _arr.size())
            _arr[i] = i;
    }
};

std::ostream &operator<<(std::ostream &os, std::vector<size_t> const &arr)
{
    std::copy(std::begin(arr), std::end(arr), std::ostream_iterator<size_t>{os, " "});
    return os;
}

int main()
{
    using namespace par;

    // пример со статичной функцией
    parallel_for(-10, 20, &suspend_func);

    std::vector<size_t> indices(1000);
    std::cout << "initial array\n"
              << indices << std::endl;
    // пример с функтором
    parallel_for(size_t{}, indices.size(), array_wrapper{indices});
    std::cout << "after the first filling\n"
              << indices << std::endl;
    // пример с лямбдой и итераторами
    parallel_for(std::begin(indices), std::end(indices), [](size_t &i)
                 { i = 1; });
    std::cout << "after the second filling\n"
              << indices << std::endl;

    return 0;
}
