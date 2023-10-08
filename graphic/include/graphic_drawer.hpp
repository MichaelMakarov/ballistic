#pragma once

#include <figure_info.hpp>

struct graphic_info
{
    figure_info figure;
    unsigned row;
    unsigned column;
};

class graphic_drawer
{
public:
    static void init(int argc, char *argv[]);
    static void show();
    static void draw(graphic_info const *infos, std::size_t count);
    static void draw(figure_info const &info);
};