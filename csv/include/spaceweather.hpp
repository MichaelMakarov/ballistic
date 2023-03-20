#pragma once
#include <ctime>

/**
 * @brief Данные космической погоды
 *
 */
struct spaceweather
{
    /**
     * @brief Квазилогарифмический планетарный среднесуточный индекс геомагнитной возмущенности (баллы)
     *
     */
    double kp;
    /**
     * @brief Среднесуточный индекс солнечной активности
     *
     */
    double f10_7;
    /**
     * @brief Средневзвешеныый индекс солнечной активности
     *
     */
    double f81;
};

/**
 * @brief Get the spaceweather object
 *
 * @param t время прошедшее с начала 1970 года (сек)
 * @return spaceweather
 */
spaceweather get_spaceweather(time_t t);