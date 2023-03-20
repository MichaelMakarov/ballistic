#pragma once
#include <times.hpp>

/**
 * @brief Вычисление плотности статической атмосферы согласно ГОСТ 1981.
 *
 * @param h высота в ГСК
 * @return double
 */
double atmosphere1981(double h);

/**
 * @brief Вычисление плотности динамической атмосферы согласно ГОСТ 2004.
 *
 * @param p координаты в ГСК (x, y, z)
 * @param h высота в ГСК
 * @param t время
 * @param sol_long долгота Солнца в ГСК (рад)
 * @param sol_incl наклонение Солнца в АСК (рад)
 * @param f10_7 среднесуточный индекс солнечной активности
 * @param f81 средневзвешеныый индекс солнечной активности
 * @param kp квазилогарифмический планетарный среднесуточный индекс геомагнитной возмущенности (баллы)
 * @return double
 */
double atmosphere2004(double const *p, double h, time_type t, double sol_long, double sol_incl,
                      double f10_7, double f81, double kp);
