#include <celestrack_managing.h>

#include <transform.h>
#include <egm.h>

#include <assertion.h>

#include <string>
#include <list>

std::list<elsetrec> read_tlegroup(std::istream& is)
{
	std::list<elsetrec> trecs;
	std::string lines[2];
	for (size_t count = 0; std::getline(is, lines[count]); count = (count + 1) % 2) {
		if (count == 1) {
			trecs.push_back(read_elsetrec(lines[0], lines[1]));
		}
	}

	return trecs;
}

elsetrec read_elsetrec(std::istream& is) {
	std::string lines[2];
	for (std::string line; std::getline(is, line); ) {
		auto beg = line.substr(0, 2);
		if (beg == "1 ") {
			lines[0] = line;
		}
		else if (beg == "2 ") {
			lines[1] = line;
		}
	}
	elsetrec et;
	double opt[3];
	SGP4Funcs::twoline2rv(lines[0].data(), lines[1].data(), 'c', 'd', 'v', gravconsttype::wgs84, opt[0], opt[1], opt[2], et);
	return et;
}

elsetrec read_elsetrec(std::string& first_line, std::string& second_line) {
	elsetrec et;
	double opt[3];
	SGP4Funcs::twoline2rv(first_line.data(), second_line.data(), 'c', 'd', 'v', gravconsttype::wgs84, opt[0], opt[1], opt[2], et);
	return et;
}


orbit_observation elsetrec_to_motion_params(elsetrec et)
{
	constexpr double wrot = ball::EGM96::angv;
	orbit_observation meas;
	int ta[5];
	double sec, ms;

	SGP4Funcs::invjday_SGP4(et.jdsatepoch, et.jdsatepochF, ta[0], ta[1], ta[2], ta[3], ta[4], sec);
	ms = std::modf(sec, &sec) * 1e3;
	meas.jd = times::make_juliandate(
		times::make_datetime(
			ta[0], 
			static_cast<unsigned short>(ta[1]), 
			static_cast<unsigned short>(ta[2]), 
			static_cast<unsigned short>(ta[3]), 
			static_cast<unsigned short>(ta[4]), 
			static_cast<unsigned short>(sec), 
			static_cast<unsigned short>(ms)
		)
	);

	ball::RV rv;
	ASSERT(
		SGP4Funcs::sgp4(et, 0, rv.data(), rv.data() + 3),
		"failed to convert elsetrec to parameters of motion"
	);

	auto sidereal_time = SGP4Funcs::gstime_SGP4(meas.jd.to_double() - 0.5);
	meas.vc = ball::ACS_to_GCS(rv, sidereal_time, wrot) * 1e3;

	return meas;
}


