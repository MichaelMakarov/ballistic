#include <model_wrapper.h>
#include <loworbit_models.h>
#include <future>
#include <iomanip>

namespace std {
	std::ostream& operator<< (std::ostream& ostr, const measurement_t& m)
	{
		ostr << times::make_datetime(m.second) << " ( ";
		for (size_t i{}; i < m.first.size(); ++i) ostr << m.first[i] << ' ';
		return ostr << ')';
	}
}

constexpr std::array<double, 7> variations{ 25.0, 25.0, 25.0, 0.25, 0.25, 0.25, 0.0016 };

ball::forecast<6> msa_handler::compute_forecast(const ball::motion_params<6>& mp, const ball::JD& tk, double sb) const
{
	MSA model{ _harmonics, sb };
	return make_forecast(mp, tk, model);
}

ball::forecast<6> mdasm_handler::compute_forecast(const ball::motion_params<6>& mp, const ball::JD& tk, double sb) const
{
	MDASM model{ _harmonics, _params, sb };
	return make_forecast(mp, tk, model);
}

base_wrapper::base_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end)
{
	_mp = mp;
	_begin = begin;
	_end = end;
	_tk = std::max_element(_begin, _end, [](const auto& left, const auto& right) { return left.second < right.second; })->second;
	_count = std::distance(_begin, _end);
}


msa_wrapper::msa_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end, double sb, size_t harmonics) : base_wrapper(mp, begin, end)
{
	_harmonics = harmonics;
	_sb = sb;
}

mdasm_wrapper::mdasm_wrapper(const ball::motion_params<6>& mp, iterator_t begin, iterator_t end, double sb, size_t harmonics, const ball::dma_params& params) : base_wrapper(mp, begin, end)
{
	_harmonics = harmonics;
	_sb = sb;
	_params = params;
}

void base_wrapper::compute(std::vector<math::array_view<3>>& residuals, std::vector<std::array<math::array_view<3>, 7>>& derivatives) const
{
	std::array<ball::motion_params<6>, 6> mparams;
	std::array<std::future<ball::forecast<6>>, 7> forecasts;
	for (size_t i{}; i < 6; ++i) {
		mparams[i] = _mp;
		mparams[i].vec[i] += variations[i];
		forecasts[i] = std::async(std::launch::async, &base_wrapper::compute_forecast, static_cast<const model_handler*>(this), mparams[i], _tk, _sb);
	}
	forecasts[6] = std::async(std::launch::async, &base_wrapper::compute_forecast, static_cast<const model_handler*>(this), _mp, _tk, _sb + variations[6]);
	auto forecast = compute_forecast(_mp, _tk, _sb);
	size_t point_index{};
	std::vector<ball::motion_params<6>> mplist(_count);
	for (auto it = _begin; it != _end; ++it, ++point_index) {
		mplist[point_index] = forecast.get_point(it->second);
		for (size_t coord_index{}; coord_index < 3; ++coord_index) {
			residuals[point_index][coord_index] = it->first[coord_index] - mplist[point_index].vec[coord_index];
		}
	}
	for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
		forecast = forecasts[param_index].get();
		for (point_index = 0; point_index < mplist.size(); ++point_index) {
			auto mp = forecast.get_point(mplist[point_index].jd);
			for (size_t coord_index{}; coord_index < 3; ++coord_index) {
				derivatives[point_index][param_index][coord_index] = (mp.vec[coord_index] - mplist[point_index].vec[coord_index]) / variations[param_index];
			}
		}
	}
}

void base_wrapper::update(math::array_view<7> corrections)
{
	for (size_t i{}; i < 6; ++i) _mp.vec[i] += corrections[i];
	_sb += corrections[6];
}
