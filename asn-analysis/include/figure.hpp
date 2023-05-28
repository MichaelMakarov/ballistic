#pragma once
#include <memory>

/**
 * @brief Проавйдер для отображения окна с графиками
 *
 */
class figure_provider
{
    static std::unique_ptr<class QApplication> app;

public:
    static void initialize(int argc, char **argv);
    static void show_residuals(double const *x1, double const *y1, double const *x2, double const *y2, std::size_t count);
};