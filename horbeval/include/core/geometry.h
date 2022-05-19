#pragma once
#include <linalg.h>
#include <vector>

/**
 * @brief Параметры поверхности
 * 
 */
struct surface_params {
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
struct object_model {
    /**
     * @brief Массив параметров поверхности тела
     * 
     */
    std::vector<surface_params> surface;
    /**
     * @brief Масса тела
     * 
     */
    double mass;
};

/**
 * @brief Параметры диска
 * 
 */
struct round_plane_info {
    /**
     * @brief Радиус, м
     * 
     */
    double rad;
    /**
     * @brief К-т отражения поверхности
     * 
     */
    double refl;
    /**
     * @brief Масса, кг
     * 
     */
    double mass;

    double square() const;
};
/**
 * @brief Создание модели диска
 * 
 * @param info параметры диска
 */
object_model make_round_plane(const round_plane_info& info);