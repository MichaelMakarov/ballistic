#pragma once
#include <loworbit_models.h>
#include <structures.h>
#include <optimize.h>
#include <forecast.h>
#include <list>
#include <streambuf>

using JD = times::juliandate;
using measurement_t = std::pair<std::array<double, 6>, JD>;
using iterator_t = std::list<measurement_t>::const_iterator;

struct model_handler {
	virtual ball::forecast<6> compute_forecast(const ball::motion_params<6>& mp, const ball::JD& tk, double sb) const = 0;
};

class msa_handler : public virtual model_handler {
protected:
	size_t _harmonics;
public:
	ball::forecast<6> compute_forecast(const ball::motion_params<6>& mp, const ball::JD& tk, double sb) const override;
};

struct mdasm_handler : public virtual model_handler {
protected:
	ball::dma_params _params;
	size_t _harmonics;
public:
	ball::forecast<6> compute_forecast(const ball::motion_params<6>& mp, const ball::JD& tk, double sb) const override;
};

class base_wrapper : public math::data_interface<7, 3>, public virtual model_handler {
protected:
	ball::motion_params<6> _mp;
	double _sb;
	iterator_t _begin, _end;
	size_t _count;
	JD _tk;
public:
	base_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end);
	const ball::motion_params<6>& params() const { return _mp; }
	double varparam() const { return _sb; }
	const ball::JD tk() const { return _tk; }
	size_t points_count() const override { return _count; }

	void compute(std::vector<math::array_view<3>>& residuals, std::vector<std::array<math::array_view<3>, 7>>& derivatives) const override;
	void update(math::array_view<7> corrections) override;
};

struct msa_wrapper : public base_wrapper, msa_handler {
	msa_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end, double sb, size_t harmonics);
};

struct mdasm_wrapper : public base_wrapper, mdasm_handler {
	mdasm_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end, double sb, size_t harmonics, const ball::dma_params& params);
};

template<typename M>
ball::forecast<6> make_forecast(const ball::motion_params<6>& mp, const JD& tk, M& model) {
	ball::forecast<6> forecast;
	forecast.run(mp, tk, [&model](const ball::RV& vc, const ball::JD& jd) { return model.acceleration(vc, jd); });
	return forecast;
}


namespace std {
	std::ostream& operator<< (std::ostream& ostr, const measurement_t& m);
}