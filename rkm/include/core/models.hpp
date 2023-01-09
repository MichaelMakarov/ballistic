#pragma once
#include <ball.hpp>
#include <maths.hpp>

using namespace math;

/**
 * @brief Базовая модель движения центра масс
 *
 */
class basic_model
{
private:
    geopotential _gpt;

public:
    interval<double> heights{1e5, 1e8};

public:
    explicit basic_model(size_t harmonics);
    vec6 operator()(const vec6 &v, const time_h &t);
};

class extbasic_model : public basic_model
{
    double _coef;

public:
    extbasic_model(size_t harmonics, double s, double m) : basic_model(harmonics), _coef{s / m} {}
    vec6 operator()(vec6 const &v, time_h const &t);
};

/**
 * @brief Параметры поверхности
 *
 */
struct surface
{
    /**
     * @brief Площадь поверхности
     *
     */
    double square;
    /**
     * @brief К-т отражения
     *
     */
    double refl;
    /**
     * @brief Нормаль к поверхности
     *
     */
    vec3 norm;
};
/**
 * @brief Модель физического тела
 *
 */
class object_model
{
public:
    /**
     * @brief Массив параметров поверхности тела
     *
     */
    std::vector<surface> surface;
    /**
     * @brief Масса тела
     *
     */
    double mass;
};

class rotator;
/**
 * @brief Расширенная модель движения центра масс
 *
 */
class extended_model : public basic_model
{
    const object_model &_obj;
    const rotator &_rot;

public:
    extended_model(size_t harmonics, const rotator &, const object_model &);
    vec6 operator()(const vec6 &v, const time_h &t);
};
