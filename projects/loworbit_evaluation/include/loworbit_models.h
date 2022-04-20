#pragma once
#include <geopotential.h>
#include <atmosphere1981.h>
#include <atmosphere2004.h>

/// <summary>
	/// A model of orbital motion.
	/// Considered: Earth's potential, simple atmosphere model.
	/// </summary>
class MSA {
private:
	ball::geopotential _potential;
	ball::atmosphere1981 _atmosphere;
	double _w, _fl, _rad;
public:
	double sball;
	double minheight{ 1e5 }, maxheight{ 5e6 };
public:
	MSA(size_t harmonics, double sb = 0);
	MSA(MSA&& other) noexcept;
	~MSA() = default;

	ball::RV acceleration(const ball::RV& vec, const ball::JD& jd);
};

/// <summary>
/// A model of orbital motion.
/// Considered: Earth's potential, a model of dynamic atmosphere (GOST 2004).
/// </summary>
class MDASM {
private:
	ball::geopotential _potential;
	ball::atmosphere2004 _atmosphere;
	ball::dma_params _params;
	double _w, _fl, _rad;
public:
	double sball;
	double minheight{ 1e5 }, maxheight{ 5e6 };
public:
	MDASM(size_t harmonics, const ball::dma_params& params, double sb = 0);
	MDASM(MDASM&& other) noexcept;
	~MDASM() = default;

	ball::RV acceleration(const ball::RV& vec, const ball::JD& jd);
};