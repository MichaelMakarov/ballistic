#pragma once
#include <cstddef>

/**
 * @brief Объект для отображения окна с графиками
 *
 */
class figure_provider
{
    int _argc;
    char **_argv;

public:
    figure_provider(int argc, char **argv) : _argc{argc}, _argv{argv}
    {
    }
    void show_residuals(double const *x1, double const *y1, double const *x2, double const *y2, std::size_t count);
};