#pragma once

#include <string>

struct axis_info
{
    std::string name;
};

struct points_info
{
    double const *x_array;
    double const *y_array;
    std::size_t count;
};

struct figure_info
{
    std::string title;
    points_info points;
    axis_info x_axis;
    axis_info y_axis;
};
