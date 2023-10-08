#pragma once

#include <string>

struct axis_info
{
    std::string name;
};

struct points_info
{
    double const *xarr;
    double const *yarr;
    std::size_t count;
};

struct figure_info
{
    std::string title;
    points_info points;
    axis_info xaxis;
    axis_info yaxis;
};
