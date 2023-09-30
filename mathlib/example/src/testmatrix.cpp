#include <auxiliaries.hpp>

namespace
{
    const matrix t{{0, 1}, {2, 3}};
    const matrix e{{1, 0}, {0, 1}};

    void _empty()
    {
        matrix m;
        throw_if_not(m.rows() == 0 && m.columns() == 0, "Incorrect dimension.");
    }

    void _init()
    {
        auto m{t};
        throw_if_not(m.rows() == t.rows(), "Incorrect rows number.");
        throw_if_not(m.columns() == t.columns(), "Incorrect columns number.");
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[r][c], "Values are not equal.");
    }

    void _add()
    {
        auto m = t + t;
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[r][c] * 2, "Values are not equal.");
    }

    void _sub()
    {
        auto m = t - t;
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == 0, "Values are not equal.");
    }

    void _mulm()
    {
        auto m = t * e;
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[r][c], "Values are not equal.");
    }

    void _mulv()
    {
        auto v = t * vector{1, 2};
        throw_if_not(v.size() == t.rows(), "Incorrect vector size.");
        throw_if_not(v[0] == 2 && v[1] == 8, "Incorrect value.");
    }

    void _mul()
    {
        auto m = t * 2;
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[r][c] * 2, "Values are not equal.");
    }

    void _div()
    {
        auto m = t / 2;
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[r][c] / 2, "Values are not equal.");
    }

    void _tr()
    {
        auto m = transpose(t);
        throw_if_not(m.rows() == t.columns() && m.columns() == t.rows(), "Incorrect dimension.");
        for (size_t r{}; r < m.rows(); ++r)
            for (size_t c{}; c < m.columns(); ++c)
                throw_if_not(m[r][c] == t[c][r], "Values are not equal.");
    }

    void _inv_eye()
    {
        auto m{e};
        auto d = inverse(m);
        throw_if_not(d == 1, "Invalid determinant.");
        for (size_t r{}; r < m.rows(); ++r)
            throw_if_not(m[r][r] == 1, "Values are not equal.");
    }

    void _inv_mat()
    {
        auto m{t};
        auto d = inverse(m);
        throw_if_not(d == -2, "Determinant must be equal to -2");
        m = m * t;
        for (size_t i{}; i < m.rows(); ++i)
        {
            throw_if_not(m[i][i] == 1, "Value must be equal to 1");
        }
    }
}

void test_matrix()
{
    _empty();
    _init();
    _add();
    _sub();
    _mul();
    _div();
    _mulv();
    _mulm();
    _tr();
    _inv_eye();
    _inv_mat();
}