#pragma once
#include <ball.h>

#include <geometry.h>

/**
 * @brief Параметры вращения
 * 
 */
struct rotational_params {
    /**
     * @brief Ось вращения
     * 
     */
	vec3 axis;
    /**
     * @brief Угловая скорость вращения, рад
     * 
     */
    double vel;
    /**
     * @brief Кватернион начального поворота
     * 
     */
	quaternion quat;
    /**
     * @brief Начальное время
     * 
     */
	time_h tn;
    /**
     * @brief Вычисление вращения на заданный момент времени 
     */
	quaternion rotation(time_h t) const;
};

/**
 * @brief Базовая модель движения центра масс
 * 
 */
class basic_motion_model {
protected:
    double _w;
    double _fl;
    double _rad;
	geopotential _gpt;
public:
    double minh{ 1e5 };
    double maxh{ 1e8 };
public:
    explicit basic_motion_model(size_t harmonics);

	vec6 acceleration(const vec6& v, const time_h& t);
};
/**
 * @brief Расширенная модель движения центра масс
 * 
 */
class extended_motion_model : public basic_motion_model {
    const object_model* _obj;
    rotational_params _rp;
public:
    extended_motion_model(size_t harmonics, const rotational_params& rp, const object_model* const obj);

	vec6 acceleration(const vec6& v, const time_h& t);
};
