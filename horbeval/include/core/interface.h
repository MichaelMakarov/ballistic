#pragma once
#include <integration.h>

#include <optimize.h>

#include <observation.h>
#include <highorbit.h>

using motion_params = integratable_point<vec6, time_h>;
using forecast = integration_interface<vec6, time_h, double>;


forecast make_forecast(const motion_params& mp, time_h tk);
forecast make_forecast(const motion_params& mp, time_h tk, const rotational_params& rp, const round_plane_info& info);

using measurement_iter = iterator_t<orbit_observation>;
/**
 * @brief Интерфейс для модели движения центра масс
 * 
 */
class linear_motion_interface {
protected:
	motion_params _mp;
	time_h _tk;
    measurement_iter _beg, _end;

public:
	linear_motion_interface(const motion_params& mp, measurement_iter beg, measurement_iter end);

    size_t points_count() const;
	const motion_params& parameters() const { return _mp; }
	time_h final_time() const { return _tk; }
};

/**
 * @brief Интерфецс для базовой модели движения
 * 
 */
class basic_linear_interface : public linear_motion_interface, public optimize_interface<6, 3> {
public:
    using linear_motion_interface::linear_motion_interface;

    size_t points_count() const override { return linear_motion_interface::points_count(); }
	void update(array_view<6> ) override;
	void compute(std::vector<array_view<3>>& , std::vector<std::array<array_view<3>, 6>>& ) const override;
};

/**
 * @brief Интерфейс для расширенной моедли движения
 * 
 */
class extended_linear_interface : public linear_motion_interface, public optimize_interface<7, 3> {
    rotational_params _rp;
    round_plane_info _plane;
public:
    extended_linear_interface(const motion_params&, const rotational_params&, const round_plane_info&, measurement_iter beg, measurement_iter end);
    size_t points_count() const override { return linear_motion_interface::points_count(); }
    void update(array_view<7> ) override;
    void compute(std::vector<array_view<3>>& , std::vector<std::array<array_view<3>, 7>>& ) const override;
    const auto& rotation_params() const { return _rp; }
    const auto& object_params() const { return _plane; }
};

using observation_iter = iterator_t<rotation_observation>;

/**
 * @brief 
 * 
 * @param mp параметры движения
 * @param tk конечное время
 * @param beg начало измерений
 * @param end конец измерений
 * @param v нач. вектор
 * @param strbuf буфер потока логирования
 * @return параметры вращения
 */
rotational_params estimate_rotation(
    const motion_params& mp, time_h tk, observation_iter beg, observation_iter end, const vec3& v,
    std::streambuf* strbuf
);