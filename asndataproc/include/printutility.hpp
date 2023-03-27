#pragma once
#include <ostream>

constexpr auto equal{" = "};
constexpr auto meter{"(м)"};
constexpr auto mps{"(м/с)"};
constexpr char const delta[]{char(0xCE), char(0x94)};
constexpr char const *vec_headers[]{"x", "y", "z", "vx", "vy", "vz", "s", "c"};
constexpr char const *vec_types[]{meter, meter, meter, mps, mps, mps, "", ""};

template <std::size_t size>
void print_vec(std::ostream &os, double const *v)
{
    for (std::size_t i{}; i < size; ++i)
    {
        os << vec_headers[i] << equal << v[i] << vec_types[i] << ' ';
    }
}

template <std::size_t size>
void print_dvec(std::ostream &os, double const *v)
{
    for (std::size_t i{}; i < size; ++i)
    {
        os << delta << vec_headers[i] << equal << v[i] << vec_types[i] << ' ';
    }
}