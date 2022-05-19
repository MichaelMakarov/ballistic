#pragma once
#include <times.h>

/**
 * @brief Измерение параметров движения центра масс
 * 
 */
struct orbit_observation {
    /**
     * @brief Вектор состояния в ГСК (x, y, z, vx, vy, vz)
     * 
     */
    double v[6];
    /**
     * @brief Время измерения
     * 
     */
	time_h t;
};
/**
 * @brief Измерение параметров вращения вокруг центра масс
 * 
 */
struct rotation_observation {
    /**
     * @brief Вектор наблюдателя в АСК (x, y, z)
     * 
     */
    double o[3];
    /**
     * @brief Время измерения
     * 
     */
	time_h t;
    /**
     * @brief Звёздная величина блеска
     * 
     */
    double s;
};

std::ostream& operator<<(std::ostream& , const orbit_observation& );
std::ostream& operator<<(std::ostream& , const rotation_observation& );

#include <vector>
#include <istream>
#include <utility>

template<typename T>
using iterator_t = typename std::vector<T>::const_iterator;

/**
 * @brief Провайдер данных
 * 
 * @tparam T тип данных
 */
template<typename T>
class provider {
protected:
    std::vector<T> _list;
public:
    /**
     * @brief Извлечение подмножества данных в соответствии с интервалом времени
     * 
     * @param tn начальное время
     * @param tk конечное время
     * @return пара итераторов на первый и последний элементы интервала соответственно
     */
	std::pair<iterator_t<T>, iterator_t<T>> retrieve(time_h tn, time_h tk) const;
    /**
     * @brief Итератор на первый элемент данных
     */
    iterator_t<T> begin() const { return _list.begin(); }
    /**
     * @brief Итератор на конец данных
     */
    iterator_t<T> end() const { return _list.end(); }
};

class orbit_observation_provider : public provider<orbit_observation> {
public:
    orbit_observation_provider(std::istream&& in);
};

class rotation_observation_provider : public provider<rotation_observation> {
public:
    rotation_observation_provider(std::istream&& ostr, std::istream& mstr);
};

#include <algorithm>

template<typename T>
inline std::pair<iterator_t<T>, iterator_t<T>> provider<T>::retrieve(time_h tn, time_h tk) const
{
    auto beg = std::lower_bound(
        std::begin(_list), std::end(_list), tn, 
        [](const T& elem, time_h ref){ return elem.t < ref; }
    );
    auto end = std::upper_bound(
        std::begin(_list), std::end(_list), tk, 
        [](time_h ref, const T& elem){ return ref < elem.t; }
    );
    return { beg, end };
}
