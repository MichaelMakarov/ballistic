#include <observation.h>

template<size_t size>
std::ostream& operator<<(std::ostream& out, const double(&arr)[size])
{
    out << "( ";
    for (auto a : arr) out << a << ' ';
    return out << ')';
}

std::ostream& operator<<(std::ostream& out, const orbit_observation& obs)
{
    return out << obs.t << ' ' << obs.v;
}

std::ostream& operator<<(std::ostream& out, const rotation_observation& obs)
{
    return out << obs.t << ' ' << obs.o;
}

#include <SGP4.h>

elsetrec read_elsetrec(char* first, char* second)
{
    elsetrec trec;
    double opt[3];
    SGP4Funcs::twoline2rv(
        first, second,
        'c', 'd', 'v', gravconsttype::wgs84, 
        opt[0], opt[1], opt[2],
        trec
    );
    return trec;
}

#include <ball.h>
#include <assertion.h>
#include <transform.h>

orbit_observation elsetrec_to_observation(elsetrec trec)
{
	constexpr double wrot = EGM96::angv;
    orbit_observation obs;
    int ta[5];
    double sec;
    SGP4Funcs::invjday_SGP4(
        trec.jdsatepoch, trec.jdsatepochF, 
        ta[0], ta[1], ta[2], ta[3], ta[4], sec
    );
    double ms = std::modf(sec, &sec) * 1e3;
	obs.t = make_time(calendar{ ta[0], ta[1], ta[2], ta[3], ta[4], int(sec), int(ms) });
    double buf[6];
    ASSERT(
        SGP4Funcs::sgp4(trec, 0, buf, buf + 3),
        "не удалось выполнить процедуру приведения tle-параметров к вектору состояния по модели SGP4"
    );
    auto st = sidereal_time_mean(obs.t);
	transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, wrot, obs.v);
    for (double& e : obs.v) e *= 1e3;// приведение к м и м/с
    return obs;
}

#include <string>

struct tle {
    std::string first, second;
};

std::istream& operator>>(std::istream& in, tle& t)
{
    if (std::getline(in, t.first)) {
        return std::getline(in, t.second);
    }
    return in;
}

#include <iterator>

orbit_observation_provider::orbit_observation_provider(std::istream&& in)
{
    tle t;
    while (in >> t) {
        _list.push_back(elsetrec_to_observation(read_elsetrec(t.first.data(), t.second.data())));
    }
    std::sort(std::begin(_list), std::end(_list), [](const auto& left, const auto& right){ return left.t < right.t; });
}

