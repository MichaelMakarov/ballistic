#include <observation.hpp>
#include <maths.hpp>
#include <ball.hpp>
#include <transform.hpp>
#include <SGP4.h>
#include <algorithm>
#include <fstream>
#include <string>

/// функции обработки ТЛЕ

elsetrec read_elsetrec(const char *first, const char *second)
{
    auto fptr = const_cast<char *>(first);
    auto sptr = const_cast<char *>(second);
    elsetrec trec;
    double opt[3];
    SGP4Funcs::twoline2rv(fptr, sptr, 'c', 'd', 'v', gravconsttype::wgs84, opt[0], opt[1], opt[2], trec);
    return trec;
}

orbit_data elsetrec_to_observation(elsetrec trec)
{
    orbit_data obs;
    int ta[5];
    double sec;
    SGP4Funcs::invjday_SGP4(
        trec.jdsatepoch, trec.jdsatepochF,
        ta[0], ta[1], ta[2], ta[3], ta[4], sec);
    double ms = std::modf(sec, &sec) * 1e3;
    obs.t = make_time(calendar{ta[0], ta[1], ta[2], ta[3], ta[4], int(sec), int(ms)});
    double buf[6]{};
    if (!SGP4Funcs::sgp4(trec, 0, buf, buf + 3))
    {
        throw std::runtime_error("не удалось выполнить процедуру приведения tle-параметров к вектору состояния по модели SGP4.");
    }
    auto st = sidereal_time_mean(obs.t);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, egm::angv, obs.v);
    for (double &e : obs.v)
        e *= 1e3; // приведение к м и м/с
    return obs;
}

struct tle
{
    std::string first, second;
};

std::istream &operator>>(std::istream &in, tle &t)
{
    if (std::getline(in, t.first))
    {
        return std::getline(in, t.second);
    }
    return in;
}

std::ifstream open_infile(std::string_view filename);

auto load_orbit_data(const std::string_view filename) -> std::vector<orbit_data>
{
    auto fin = open_infile(filename);
    std::vector<orbit_data> list;
    std::transform(
        std::istream_iterator<tle>{fin}, std::istream_iterator<tle>{}, std::back_inserter(list),
        [](const tle &t)
        { return elsetrec_to_observation(read_elsetrec(t.first.data(), t.second.data())); });
    std::sort(std::begin(list), std::end(list), [](auto &left, auto &right)
              { return left.t < right.t; });
    return list;
}
