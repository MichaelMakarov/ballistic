#pragma once
#include <qdatetime.h>

#include <observation.h>
#include <geometry.h>

#include <linalg.h>

/**
 * @brief Параметры невязок
 * 
 */
struct residuals_info {
    /**
     * @brief Массив невязок
     * 
     */
    std::vector<double> array;
    /**
     * @brief Среднее значение
     * 
     */
    double mean;
    /**
     * @brief СКО
     * 
     */
    double std;
};
/**
 * @brief Параметры мерного интервала
 * 
 */
struct interval_info {
    /**
     * @brief Итератор на первый элемент орбитальных измерений
     * 
     */
    iterator_t<orbit_observation> orb_begin;
    /**
     * @brief Итератор на конец орбитальных измерений
     * 
     */
    iterator_t<orbit_observation> orb_end;
    /**
     * @brief Итератор на первый элемент измерений вращения
     * 
     */
    iterator_t<rotation_observation> rot_begin;
    /**
     * @brief Итератор на конец измерений вращения
     * 
     */
    iterator_t<rotation_observation> rot_end;
};
/**
 * @brief Параметры движения
 * 
 */
struct motion_info {
    /**
     * @brief Вектор состояния
     * 
     */
    vec6 v;
    /**
     * @brief Невязки
     * 
     */
    residuals_info r;
};
/**
 * @brief Параметры вращения
 * 
 */
struct rotation_info {
    /**
     * @brief Ось
     * 
     */
    vec3 axis;
    /**
     * @brief Угловая скорость [рад/с]
     * 
     */
    double vel;
};
/**
 * @brief Параметры модели КА
 * 
 */
struct object_info {
    /**
     * @brief Радиус поверхности
     * 
     */
    double rad;
};

enum struct computation_stage {
    zero,
    first,
    second,
    third,
    fourth,
    fifth
};

class computation {
public:
    void reset();
    void interval_info_updated() { _stage = computation_stage::first; }
    void rotation_info_updated() { _stage = computation_stage::fourth; }
    void init_motion_info_updated() { _stage = computation_stage::second; }
    void prev_motion_info_updated() { _stage = computation_stage::third; }
    void next_motion_info_updated() { _stage = computation_stage::fifth; }
    std::string to_string() const;

    interval_info interval;
    rotation_info rotation;
    motion_info init_motion;
    motion_info prev_motion;
    motion_info next_motion;
    object_info prev_object;
    object_info next_object;
private:
    computation_stage _stage = computation_stage::zero;
};

/**
 * @brief Настройки проекта
 * 
 */
struct project_settings {
    /**
     * @brief Путь к файлу ГПЗ
     * 
     */
    std::string gptpath;
    /**
     * @brief Путь к файлу TLE
     * 
     */
    std::string tlepath;
    /**
     * @brief Путь к файлу с обсерваториями
     * 
     */
    std::string obspath;
    /**
     * @brief Путь к файлу с измерениями
     * 
     */
    std::string mespath;
    /**
     * @brief Параметры КА
     * 
     */
    round_plane_info object;
};

extern project_settings gl_settings;

QDateTime from_time(time_h t);

class mainmodel {
public:
    mainmodel() = default;

    void read_geopotential(const QString& filename);
    void read_tle(const QString& filename);
    void read_measurements(const QString& mfilename, const QString& ofilename);
    void set_interval(const QDateTime& tn, const QDateTime& tk);
    void compute(computation* const info, const QString& filename) const;
    orbit_observation_provider* tle_provider() { return _orbprovider.get(); }
    rotation_observation_provider* obs_provider() { return _rotprovider.get(); }
    QDateTime tn() const;
    QDateTime tk() const;
private:
    std::unique_ptr<orbit_observation_provider> _orbprovider;
    std::unique_ptr<rotation_observation_provider> _rotprovider;
    time_h _tn, _tk;
};
