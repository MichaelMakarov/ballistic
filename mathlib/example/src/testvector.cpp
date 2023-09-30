#include <auxiliaries.hpp>
#include <cmath>

namespace
{
    const vector t{1, -2, 3, 2, -1};

    void _empty()
    {
        throw_if_not(vector().size() == 0, "Vector size must be equal to 0.");
    }

    void _init()
    {
        auto v{t};
        throw_if_not(v.size() == t.size(), "Sizes of the vectors are not equal.");
        for (size_t i{}; i < v.size(); ++i)
        {
            throw_if_not(v[i] == t[i], "Vectors are not the same.");
        }
    }

    void _add()
    {
        auto v = t + t;
        for (size_t i{}; i < v.size(); ++i)
        {
            throw_if_not(v[i] == t[i] + t[i], "Incorrect add.");
        }
    }

    void _sub()
    {
        auto v = t - t;
        for (auto e : v)
        {
            throw_if_not(e == 0, "Incorrect sub.");
        }
    }

    void _mul()
    {
        auto v = t * 2;
        for (size_t i{}; i < v.size(); ++i)
        {
            throw_if_not(v[i] == t[i] * 2, "Incorrect mul.");
        }
    }

    void _div()
    {
        auto v = t / 2;
        for (size_t i{}; i < v.size(); ++i)
        {
            throw_if_not(v[i] == t[i] / 2, "Incorrect div.");
        }
    }

    void _dot()
    {
        throw_if_not(t * t == 19, "Incorrect mul of vectors.");
    }

    void _len()
    {
        throw_if_not(t.length() == std::sqrt(t * t), "Incorrect length.");
    }
}

void test_vector()
{
    _empty();
    _init();
    _add();
    _sub();
    _mul();
    _div();
    _dot();
    _len();
}
