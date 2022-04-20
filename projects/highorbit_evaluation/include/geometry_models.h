#pragma once
#include <highorbit_models.h>

/// <summary>
/// Computes object parameters of square plane of specified mass, width and height. 
/// </summary>
object_model make_square_plane(
	double mass, 
	double width, 
	double height
);

/// <summary>
/// Параметры диска
/// </summary>
struct round_plane_info {
	double reflection;		// к-т отражения поверхности
	double mass;			// масса в кг
	double radius;			// радиус в м
	math::vec3 center;		// смещение центра масс относительно центра
	math::vec3 normal;		// нормаль 

	double square() const;
};

std::ostream& operator<<(std::ostream& out, const round_plane_info& plane_info);
/// <summary>
/// Computes object parameters of round plane of specified mass, radius and center of mass. 
/// </summary>
object_model make_round_plane(
	const round_plane_info& plane_info
);
