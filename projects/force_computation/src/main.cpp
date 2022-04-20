#include <iostream>
#include <fstream>
#include <iomanip>

#include <geopotential.h>
#include <solar_system.h>

#include <mathconstants.h>
#include <assertion.h>
#include <transform.h>

std::ofstream open_output_file(const std::string& filename);
std::ifstream open_input_file(const std::string& filename);

int main() {
	constexpr double rad{ 3e7 };
	constexpr double dangle{ math::PI * 1e-1 };

	try {
		ball::EGM96::load_harmonics(open_input_file("egm96.txt"));
		auto logout = open_output_file("forces.txt");
		auto dt = times::datetime::now();
		auto jd = times::make_juliandate(dt);
		double sidt = ball::sidereal_time_mean(jd);

		logout << std::setprecision(16);
		logout << dt << std::endl;
		logout << "height = " << (rad - ball::EGM96::rad) * 1e-3 << " km" << std::endl;

		for (double angle{}; angle < math::PI2; angle += math::PI * 1e-1) {
			math::vec3 coord{ rad * std::cos(angle), rad * std::sin(angle) };

			logout << "longitude = " << angle << std::endl;

			auto sun = ball::ACS_to_GCS(ball::ort_to_sph(ball::solar_model::position(jd)), sidt);
			logout << "solar acceleration " << 
				ball::acceleration_by_masspoint(
					coord[0], coord[1], coord[2], 
					sun[0], sun[1], sun[2], 
					ball::solar_model::mu()
				) << std::endl;

			auto moon = ball::ACS_to_GCS(ball::ort_to_sph(ball::lunar_model::position(jd)), sidt);
			logout << "lunar acceleration " <<
				ball::acceleration_by_masspoint(
					coord[0], coord[1], coord[2],
					moon[0], moon[1], moon[2],
					ball::lunar_model::mu()
				) << std::endl;

			logout << "geopotential: " << std::endl;
			for (size_t i{}; i < 100; ++i) {
				ball::geopotential gpt{ ball::EGM96{}, i };
				logout << 
					"harmonics count = " << i << 
					" accelecration = " << 
					gpt.acceleration(coord[0], coord[1], coord[2]) << 
					std::endl;
			}
		}
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}


	return 0;
}

std::ofstream open_output_file(const std::string& filename)
{
	std::ofstream fout{ filename };
	ASSERT(
		fout.is_open(),
		"failed to open file " + filename
	);
	return fout;
}

std::ifstream open_input_file(const std::string& filename)
{
	std::ifstream fin{ filename };
	ASSERT(
		fin.is_open(),
		"failed to open file " + filename
	);
	return fin;
}
