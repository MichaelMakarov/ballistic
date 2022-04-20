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
/// ��������� �����
/// </summary>
struct round_plane_info {
	double reflection;		// �-� ��������� �����������
	double mass;			// ����� � ��
	double radius;			// ������ � �
	math::vec3 center;		// �������� ������ ���� ������������ ������
	math::vec3 normal;		// ������� 

	double square() const;
};

std::ostream& operator<<(std::ostream& out, const round_plane_info& plane_info);
/// <summary>
/// Computes object parameters of round plane of specified mass, radius and center of mass. 
/// </summary>
object_model make_round_plane(
	const round_plane_info& plane_info
);
