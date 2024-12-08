#include <iostream>
#include <memory>

struct test_deleter
{
    void operator()(void* p) const
    {
        std::cout << "deleter" << std::endl;
        delete p;
    }
};

int main()
{
    int* a = new int;
    std::shared_ptr<int> p(a, test_deleter());

    auto deleter = [](int* p)
    {
        std::cout << "lambda deleter" << std::endl;
        delete p;
    };

    int* b = new int;
    std::shared_ptr<int> p2(b, deleter);
}
