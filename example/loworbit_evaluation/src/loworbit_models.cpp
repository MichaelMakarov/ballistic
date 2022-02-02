#include <loworbit_models.h>
#include <arithmetics.h>
#include <linalg.h>
#include <solar_system.h>
#include <transform.h>
#include <format>


using namespace ball;

MSA::MSA(size_t harmonics, double sb) :	_potential{ EGM96{}, harmonics }
{
	_potential = geopotential(EGM96{}, harmonics);
	_w = EGM96::angv;
	_fl = EGM96::flat;
	_rad = EGM96::rad;
	sball = sb;
}

MSA::MSA(MSA&& other) noexcept 
{
	_w = other._w;
	_fl = other._fl;
	_rad = other._rad;
	sball = other.sball;
	minheight = other.minheight;
	maxheight = other.maxheight;
	_potential = std::move(other._potential);
	_atmosphere = std::move(other._atmosphere);
}

RV MSA::acceleration(const RV& vc, const JD& jd) 
{
	double w2 = math::sqr(_w);
	double h = height_above_ellipsoid(vc[0], vc[1], vc[2], _rad, _fl);
	if (h < minheight || h > maxheight) throw std::runtime_error(std::format("height = {} out of bounds [{}, {}]", h, minheight, maxheight));

	auto potv = _potential.acceleration(vc[0], vc[1], vc[2]);
	double density = _atmosphere.density(h);
	// atmosphere deceleration a = v * s * density, 
	// s - a ballistic coefficient,
	// v - a velocity of the vehicle,
	// density - a density of the atmosphere
	double atmv = density * this->sball * length(math::slice<3, 5>(vc));

	return RV{
		vc[3], vc[4], vc[5],
		potv[0] + w2 * vc[0] + 2 * _w * vc[4] - atmv * vc[3],
		potv[1] + w2 * vc[1] - 2 * _w * vc[3] - atmv * vc[4],
		potv[2] - atmv * vc[5]
	};
}

MDASM::MDASM(size_t harmonics, const ball::dma_params& params, double sb)
{
	_potential = geopotential{ EGM96{}, harmonics };
	_w = EGM96::angv;
	_fl = EGM96::flat;
	_rad = EGM96::rad;
	sball = sb;
	_params = params;
}

MDASM::MDASM(MDASM&& other) noexcept
{
	_w = other._w;
	_fl = other._fl;
	_rad = other._rad;
	sball = other.sball;
	_params = other._params;
	minheight = other.minheight;
	maxheight = other.maxheight;
	_potential = std::move(other._potential);
	_atmosphere = std::move(other._atmosphere);
}

RV MDASM::acceleration(const RV& vc, const JD& jd) 
{
	double w2 = math::sqr(_w);
	double h = height_above_ellipsoid(vc[0], vc[1], vc[2], _rad, _fl);
	if (h < minheight || h > maxheight) throw std::runtime_error(std::format("height = {} out of bounds [{}, {}]", h, minheight, maxheight));

	double sidt = sidereal_time_mean(jd);
	auto solsph = solar_model::position(jd);
	auto solort = ACS_to_GCS(sph_to_ort(solsph), sidt);
	auto lunsph = lunar_model::position(jd);
	auto lunort = ACS_to_GCS(sph_to_ort(lunsph), sidt);

	double density = _atmosphere.density(h, vc[0], vc[1], vc[2], jd, std::atan2(solort[1], solort[0]), solsph[1], _params);
	// deceleration by atmosphere
	double atmv = density * this->sball * length(math::slice<3, 5>(vc));
	// acelerations by geopotential, sun and moon
	auto potv = _potential.acceleration(vc[0], vc[1], vc[2]);
	potv += acceleration_by_masspoint(vc[0], vc[1], vc[2], solort[0], solort[1], solort[2], solar_model::mu());
	potv += acceleration_by_masspoint(vc[0], vc[1], vc[2], lunort[0], lunort[1], lunort[2], lunar_model::mu());

	return RV{
		vc[3], vc[4], vc[5],
		potv[0] + w2 * vc[0] + 2 * _w * vc[4] - atmv * vc[3],
		potv[1] + w2 * vc[1] - 2 * _w * vc[3] - atmv * vc[4],
		potv[2] - atmv * vc[5]
	};
}