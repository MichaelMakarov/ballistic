#pragma once

struct graphic_info
{
    double *x;
    double *y;
    unsigned count;
    char const *xlbl, *ylbl;
    char const *title;
};

struct graphic_app
{
    static void init(int argc, char **argv);
    static void show(graphic_info const *infos, unsigned count);
};
