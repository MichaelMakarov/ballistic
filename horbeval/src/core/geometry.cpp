#include <geometry.h>
#include <conversion.h>
#include <arithmetics.h>

double round_plane_info::square() const
{
	return PI * sqr(rad);
}


object_model make_round_plane(const round_plane_info& info)
{
    object_model obj;
    obj.mass = info.mass;
    obj.surface.resize(2);

    for (auto& face : obj.surface) {
        face.refl = info.refl;
        face.square = info.square();
    }
    obj.surface[0].norm = { 0, 0, 1 };
    obj.surface[1].norm = -obj.surface[0].norm;

    return obj;
}
