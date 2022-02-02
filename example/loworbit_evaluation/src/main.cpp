#include <auxilliares.h>
#include <dataprovider.h>
#include <model_wrapper.h>
#include <stopwatch.h>
#include <iterator>
#include <iostream>
#include <iomanip>

constexpr int DELTA_SECONDS{ -18 };
 
struct residual {
	times::datetime dt;
	double pos{}, vel{};
};


/// <summary>
/// Loads compute settings from file.
/// </summary>
void read_compute_settings(const std::string& filename, compute_settings& settings);
/// <summary>
/// Loads potential harmonics from file.
/// </summary>
void read_potential_harmonics(const std::string& filename);
/// <summary>
/// Loads an amount of measurements from file.
/// </summary>
void read_measurements(std::istream& istr, const std::string& dtformat, std::list<measurement_t>& measurements, ball::motion_params<6>& mp);
/// <summary>
/// Performs the computation and model evaluation.
/// </summary>
void run_evaluation_model(base_wrapper* modelptr, std::streambuf* bufptr, iterator_t begin, iterator_t end);
/// <summary>
/// Computes the residuals.
/// </summary>
void compute_residuals(iterator_t begin, iterator_t end, const ball::forecast<6>& forecast, std::ostream& out);

int main() {
	std::string dtformat = "yyyy-MM-dd hh:mm:ss.fff";

	compute_settings settings;
	try {
		std::cout << "load computational settings..." << std::endl;
		read_compute_settings("settings.txt", settings);
	}
	catch (const std::exception& ex) {
		std::cout << "failed to load compute settings: " << ex.what() << std::endl;
		return 1;
	}

	try {
		std::cout << "load potential harmonics..." << std::endl;
		read_potential_harmonics(settings.gravity_file);
	}
	catch (const std::exception& ex) {
		std::cout << "failed to load potential harmonics: " << ex.what() << std::endl;
		return 1;
	}

	std::list<measurement_t> measurements;
	ball::motion_params<6> mp;
	try {
		std::cout << "load measurements..." << std::endl;
		auto fin = open_infile(settings.measuring_file);
		read_measurements(fin, dtformat, measurements, mp);
	}
	catch (const std::exception& ex) {
		std::cout << "failed to load measurements: " << ex.what() << std::endl;
		return 1;
	}

	std::shared_ptr<base_wrapper> wrapper_ptr;
	if (settings.model == model_type::MDASM_MODEL) {
		space_weather weather;
		try {
			std::cout << "load spaceweather data..." << std::endl;
			auto spwdata = load_spaceweather_data(settings.spaceweather_file);
			weather = spwdata[times::make_datetime(settings.datetime)];
		}
		catch (const std::exception& ex) {
			std::cout << "failed to get spaceweather data for " << settings.datetime << " " << ex.what() << std::endl;
			return 1;
		}
		ball::dma_params dma;
		dma.f10_7 = weather.F10_7;
		dma.f81 = weather.F81;
		dma.kp = weather.Kpsum;
		wrapper_ptr = std::make_unique<mdasm_wrapper>(mp, measurements.begin(), measurements.end(), 0, settings.harmonics, dma);
	}
	else {
		wrapper_ptr = std::make_unique<msa_wrapper>(mp, measurements.begin(), measurements.end(), 0, settings.harmonics);
	}

	try {
		std::cout << "start computation..." << std::endl;
		auto logout = open_outfile(settings.output_file);
		std::streambuf* bufptr{ nullptr };
		if (logout.is_open()) bufptr = logout.rdbuf();
		run_evaluation_model(wrapper_ptr.get(), bufptr, measurements.begin(), measurements.end());
	}
	catch (const std::exception& ex) {
		std::cout << "failed to evaluate model: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}

void read_compute_settings(const std::string& filename, compute_settings& settings)
{
	open_infile(filename) >> settings;
}

void read_potential_harmonics(const std::string& filename)
{
	auto fin = open_infile(filename);
	ball::load_harmonics<ball::egm_type::EGM96>(fin);
}

void read_measurements(std::istream& is, const std::string& dtformat, std::list<measurement_t>& measurements, ball::motion_params<6>& mp)
{
	std::string date, time;
	std::array<double, 6> vec;
	while (!is.eof()) {
		is >> date >> time;
		for (size_t i{}; i < vec.size(); ++i) is >> vec[i];
		measurements.push_back(std::make_pair(vec, times::make_juliandate(times::make_datetime(date + ' ' + time, dtformat)).add_seconds(DELTA_SECONDS)));
	}
	auto first_m = std::min_element(measurements.begin(), measurements.end(), [](const auto& left, const auto& right) { return left.second < right.second; });
	mp.jd = first_m->second;
	std::copy(first_m->first.begin(), first_m->first.end(), mp.vec.begin());
	mp.loop = 1;
}

void run_evaluation_model(base_wrapper* modelptr, std::streambuf* bufptr, iterator_t begin, iterator_t end)
{
	std::ostream logout{ bufptr };
	auto beg = begin;
	logout << "*****MEASUREMENTS*****" << std::endl;
	for (size_t i{ 1 }; begin != end; ++begin, ++i) logout << std::setw(6) << i << " " << *begin << std::endl;
	logout << std::endl;
	logout << "*****INITIAL PARAMETERS OF MOTION*****" << std::endl;
	logout << modelptr->params() << " s = " << modelptr->varparam() << std::endl;

	times::stopwatch sw;
	std::cout << "orbit evaluation..." << std::endl;
	sw.start();
	size_t iterations = math::newton_raphson(*modelptr, bufptr);
	sw.finish();
	std::cout << "computation time " << sw.duration() << " iterations count = " << iterations << std::endl;

	logout << "*****FINAL PARAMETERS OF MOTION*****" << std::endl;
	logout << modelptr->params() << " s = " << modelptr->varparam() << std::endl;
	logout << std::endl;

	std::cout << "computation residuals..." << std::endl;
	auto forecast = modelptr->compute_forecast(modelptr->params(), modelptr->tk(), modelptr->varparam());
	compute_residuals(beg, end, forecast, logout);
}

void compute_residuals(iterator_t begin, iterator_t end, const ball::forecast<6>& forecast, std::ostream& out)
{
	double meandr{};
	size_t count{};
	for (; begin != end; ++begin, ++count) {
		auto mp = forecast.get_point(begin->second);
		residual resid;
		for (size_t j{ 0 }; j < 3; ++j) resid.pos += math::sqr(mp.vec[j] - begin->first[j]);
		for (size_t j{ 3 }; j < 6; ++j) resid.vel += math::sqr(mp.vec[j] - begin->first[j]);
		resid.pos = std::sqrt(resid.pos);
		resid.vel = std::sqrt(resid.vel);
		resid.dt = times::make_datetime(mp.jd);
		out << std::setw(6) << count << ") " << resid.dt << " dr = " << resid.pos << " dv = " << resid.vel << std::endl;
		meandr += resid.pos;
	}
	out << "mean dr = " << meandr / count << std::endl;
}

