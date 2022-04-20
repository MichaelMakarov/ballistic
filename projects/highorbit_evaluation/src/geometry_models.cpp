#include <geometry_models.h>

#include <mathconstants.h>
#include <arithmetics.h>

object_model make_square_plane(
	double mass, 
	double width, 
	double height
) {
	double width_sqr{ width * width }, height_sqr{ height * height };
	object_model obj;
	obj.mass = mass;
	obj.inertia[0] = mass * (width_sqr + height_sqr) / 16;
	obj.inertia[1] = mass * height_sqr / 12;
	obj.inertia[2] = mass * width_sqr / 12;

	obj.surface.resize(6);
	for (auto& surf : obj.surface) {
		surf.refl = 0.25;
		surf.square = 0.0;
	}
	// a face along the first axis
	obj.surface[0].square = width * height;
	obj.surface[0].norm = math::vec3{ 1, 0, 0 };
	obj.surface[0].center = math::vec3{ 0, 0, 0 };
	// an opposite face
	obj.surface[1].square = width * height;
	obj.surface[1].norm = -obj.surface[0].norm;
	obj.surface[1].center = math::vec3{ 0, 0, 0 };
	// a face along the second axis (along width)
	obj.surface[2].norm = math::vec3{ 0, 1, 0 };
	obj.surface[2].center[1] = width / 2;
	// an opposite face
	obj.surface[3].norm = -obj.surface[2].norm;
	obj.surface[3].center[1] = -width / 2;
	// a face along the third axis (along height) 
	obj.surface[4].norm = math::vec3{ 0, 0, 1 };
	obj.surface[4].center[2] = height / 2;
	// an opposite face
	obj.surface[5].norm = -obj.surface[4].norm;
	obj.surface[5].center[2] = -height / 2;

	return obj;
}

double round_plane_info::square() const
{
	return math::PI * math::sqr(radius);
}

object_model make_round_plane(
	const round_plane_info& plane_info
) 
{
	double radius_sqr = math::sqr(plane_info.radius);
	object_model obj;

	// inertia definition
	obj.mass = plane_info.mass;
	obj.inertia[2] = 0.5 * plane_info.mass * radius_sqr;	// around z axis
	obj.inertia[0] = obj.inertia[1] = 0.5 * obj.inertia[2];	// around x and y axes

	double 
		cxsqr = math::sqr(plane_info.center[0]),
		cysqr = math::sqr(plane_info.center[1]), 
		czsqr = math::sqr(plane_info.center[2]);

	obj.inertia[0] += plane_info.mass * (cysqr + czsqr);
	obj.inertia[1] += plane_info.mass * (cxsqr + czsqr);
	obj.inertia[2] += plane_info.mass * (cxsqr + cysqr);

	// surface parameters definition
	obj.surface.resize(2);

	// a face in front of z axis
	obj.surface[0].center = plane_info.center;
	obj.surface[0].square = math::PI * radius_sqr;
	obj.surface[0].norm = math::vec3{ 0, 0, 1 };
	obj.surface[0].refl = plane_info.reflection;
	// a face behind z axis
	obj.surface[1].center = plane_info.center;
	obj.surface[1].square = obj.surface[0].square;
	obj.surface[1].norm = -obj.surface[0].norm;
	obj.surface[1].refl = plane_info.reflection;

	return obj;
}


std::ostream& operator<<(std::ostream& out, const round_plane_info& plane_info)
{
	return out <<
		"mass = " << plane_info.mass << " kg " <<
		"radius = " << plane_info.radius << " m " <<
		"refl.coef. = " << plane_info.reflection <<
		" center of mass " << plane_info.center;
}
