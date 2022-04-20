#include <structure_type.h>

std::ostream& operator<< (std::ostream& out, const orbit_observation& m) 
{
	return out << times::make_datetime(m.jd) << ' ' << m.vc;
}

std::ostream& operator<< (std::ostream& out, const rotational_observation& m)
{
	return out << 
		times::make_datetime(m.jd) << ' ' << m.vc << 
		" stellar magn = " << m.magnitude <<
		" observatory " << m.obs;
}