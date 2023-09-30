#include <auxiliaries.hpp>
#include <cmath>

namespace
{
    constexpr vec3 v1{1, 0, 0}, v2{0, 1, 0};
    constexpr quaternion t1{1, 2, 3, 4};
    quaternion t2{v1, v2};

    void _empty()
    {
        constexpr quaternion q;
        static_assert(q.s() == 1, "Incorrect value.");
        static_assert(q.x() == 0 && q.y() == 0 && q.z() == 0, "Incorrect value.");
    }

    void _init()
    {
		double sin_pi_4 = std::sin(pi * 0.25);
        static_assert(t1.s() == 4 && t1.x() == 1 && t1.y() == 2 && t1.z() == 3, "Incorrect init.");
        constexpr auto q2{t1};
        static_assert(t1.s() == q2.s() && t1.x() == q2.x() && t1.y() == q2.y() && t1.z() == q2.z(), "Incorrect init.");
		throw_if_not(t2.x() == 0 && t2.y() == 0 && is_equal(t2.z(), sin_pi_4) && is_equal(t2.s(), t2.z()), "Incorrect init.");
        quaternion q4({0, 0, 1}, pi * 0.5);
        throw_if_not(q4.s() == t2.s() && q4.x() == t2.x() && q4.y() == t2.y() && q4.z() == t2.z(), "Incorrect init.");
    }

    void _add()
    {
        constexpr auto q = t1 + t1;
        static_assert(q.s() == t1.s() * 2 && q.x() == t1.x() * 2 && q.y() == t1.y() * 2 && q.z() == t1.z() * 2, "Incorrect value.");
    }

    void _sub()
    {
        constexpr auto q = t1 - t1;
        static_assert(q.s() == 0 && q.x() == 0 && q.y() == 0 && q.z() == 0, "Incorrect value.");
    }

    void _mul()
    {
        constexpr auto q = t1 * 2;
        static_assert(q.s() == t1.s() * 2 && q.x() == t1.x() * 2 && q.y() == t1.y() * 2 && q.z() == t1.z() * 2, "Incorrect value.");
    }

    void _div()
    {
        constexpr auto q = t1 / 2;
        static_assert(q.s() == t1.s() / 2 && q.x() == t1.x() / 2 && q.y() == t1.y() / 2 && q.z() == t1.z() / 2, "Incorrect value.");
    }

    void _dot()
    {
        static_assert(dot(t1, t1) == 30, "Incorrect valur.");
    }

    void _conj()
    {
        constexpr auto q = conj(t1);
        static_assert(q.s() == t1.s() && q.x() == -t1.x() && q.y() == -t1.y() && q.z() == -t1.z(), "Incorrect value.");
    }

    void _inv()
    {
        constexpr auto n = dot(t1, t1);
        constexpr auto i = inverse(t1);
        static_assert(i.s() == t1.s() / n && i.x() == -t1.x() / n && i.y() == -t1.y() / n && i.z() == -t1.z() / n, "Incorrect value.");
    }

    void _mulq()
    {
        constexpr auto q = mul(quaternion{1, 0, 0, 0}, quaternion{0, 1, 0, 0});
        static_assert(q.s() == 0 && q.x() == 0 && q.y() == 0 && q.z() == 1, "Incorrect value.");
    }

    void _rot()
    {
        auto v = t2.rotate(v1);
		throw_if_not(is_equal(v[0], v2[0]) && is_equal(v[1], v2[1]) && is_equal(v[2], v2[2]), "Incorrect value.");
    }
}

void test_quaternion()
{
    _init();
    _rot();
}
