#include <ball.hpp>
#include <transform.hpp>
#include <maths.hpp>
#include <cmath>

using namespace math;

double time_to_jd(int64_t t);
double jc2000(double jd);
double jc2000(int64_t t)
{
    // домножаем на 1000, чтобы перевести в миллисекунды
    return jc2000(time_to_jd(t * 1000));
}

void sum_of(double c1, double s1, double c2, double s2, double &c, double &s)
{
    c = c1 * c2 - s1 * s2;
    s = s1 * c2 + c1 * s2;
}

void solar_model::coordinates(int64_t t, double *ort, double *sph)
{
    const double T = jc2000(t);
    // solar average longitude
    double L = sec_to_rad(1009677.85 + (100 * 1296000 + 2771.27 + 1.089 * T) * T);
    // solar perigee average longitude
    double lc = sec_to_rad(1018578.046 + (6190.046 + (1.666 + 0.012 * T) * T) * T);
    // the Earth's orbit eccentricity
    double e = 0.0167086342 - (0.000004203654 + (0.00000012673 + 0.00000000014 * T) * T) * T;
    // average ecliptic inclination
    double ecl = sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * T) * T) * T);
    // ecliptic average longitude of lunar ascending node
    double omega = sec_to_rad(450160.280 - (5 * spr + 482890.539 - (7.455 + 0.008 * T) * T) * T);
    // long periodic nutation of the Earth
    double psi = sec_to_rad(-17.1996 * std::sin(omega));
    // solar longitude
    double longitude = L + 2 * e * std::sin(L - lc) + 1.25 * e * e * std::sin(2 * (L - lc));
    double sinl{std::sin(longitude)};
    double cosl{std::cos(longitude)};
    double sine{std::sin(ecl)};
    double cose{std::cos(ecl)};
    double pos[3];
    pos[1] = std::atan(sinl * sine / std::sqrt(cosl * cosl + sinl * sinl * cose * cose));
    pos[2] = std::atan(sinl / cosl * cose);
    if (pos[1] * pos[2] < 0)
        pos[2] += pi;
    // mean distance from the sun to the earth
    constexpr double ac{1.4959787e11};
    double cosllc = std::cos(L - lc);
    pos[0] = ac * (1 - e * (cosllc - e * 0.25 * (1 - cosllc)));
    // a constant of the aberration
    constexpr double hi{sec_to_rad(20.49552)};
    // corrections caused by nutation
    pos[2] += 0.061165 * psi - hi;
    pos[1] += hi * sine * cosl;

    if (sph)
        std::copy(pos, pos + 3, sph);
    if (ort)
        transform<abs_cs, sph_cs, abs_cs, ort_cs>::forward(pos, ort);
}

void lunar_model::coordinates(int64_t t, double *const out)
{
    const double T = jc2000(t);
    // radius of Earth's equator
    constexpr double r{6378136};
    // средняя аномалия Луны
    double la = sec_to_rad((485866.733 + (1717915922.633 + 715922.633 + (31.31 + 0.064 * T) * T) * T));
    // средняя аномалия Солнца
    double sa = sec_to_rad((1287099.804 + (129596581.224 - (0.577 + 0.012 * T) * T) * T));
    // средний аргумент широты Луны
    double f = sec_to_rad((335778.877 + (1739527263.137 - (13.257 - 0.011 * T) * T) * T));
    // разность средних долгот Луны и Солнца
    double d = sec_to_rad((1072261.307 + (1602961601.328 - (6.891 - 0.019 * T) * T) * T));
    // координаты Луны
    double pos[3]{};
    // ecliptic latitude
    pos[1] = sec_to_rad(
        18461.48 * std::sin(f) +
        1010.18 * std::sin(la + f) -
        999.69 * std::sin(f - la) -
        623.65 * std::sin(f - 2 * d) +
        199.48 * std::sin(f + 2 * d - la) -
        166.57 * std::sin(la + f - 2 * d) +
        117.26 * std::sin(f + 2 * d) +
        61.91 * std::sin(2 * la + f) -
        33.35 * std::sin(f - 2 * d - la) -
        31.76 * std::sin(f - 2 * la) -
        29.68 * std::sin(sa + f - 2 * d) +
        15.125 * std::sin(la + f + 2 * d) -
        15.56 * std::sin(2 * (la - d) + f));
    // ecliptic longitude
    pos[2] = sec_to_rad(
        // mean longitude
        785939.157 + (1336 * spr + 1108372.598 + (5.802 + 0.019 * T) * T) * T +
        // delta longitude
        22639.5 * std::sin(la) -
        4586.42 * std::sin(la - 2 * d) +
        2369.9 * std::sin(2 * d) +
        769.01 * std::sin(2 * la) -
        668.11 * std::sin(sa) -
        411.6 * std::sin(2 * f) -
        211.65 * std::sin(2 * (la - d)) -
        205.96 * std::sin(la + sa - 2 * d) +
        191.95 * std::sin(la + 2 * d) -
        165.14 * std::sin(sa - 2 * d) +
        147.69 * std::sin(la - sa) -
        125.15 * std::sin(d) -
        109.66 * std::sin(la + sa) -
        55.17 * std::sin(2 * (f - d)) -
        45.1 * std::sin(sa + 2 * f) +
        39.53 * std::sin(la - 2 * f) -
        38.42 * std::sin(la - 4 * d) +
        36.12 * std::sin(3 * la) -
        30.77 * std::sin(2 * la - 4 * d) +
        28.47 * std::sin(la - sa - 2 * d) -
        24.42 * std::sin(sa + 2 * d) +
        18.6 * std::sin(la - d) +
        18.02 * std::sin(sa - d));
    pos[2] = fit_round(pos[2]);
    // paralax
    double paralax = sec_to_rad(
        3422.7 +
        186.539 * std::cos(la) +
        34.311 * std::cos(la - 2 * d) +
        28.233 * std::cos(2 * d) +
        10.165 * std::cos(2 * la) +
        3.086 * std::cos(la + 2 * d) +
        1.92 * std::cos(sa - 2 * d) +
        1.445 * std::cos(la + sa - 2 * d) +
        1.154 * std::cos(la - sa) -
        0.975 * std::cos(d) -
        0.95 * std::cos(la + sa) -
        0.713 * std::cos(la - 2 * f) +
        0.6215 * std::cos(3 * la) +
        0.601 * std::cos(la - 4 * d));
    pos[0] = r / paralax;
    // average ecliptic inclination
    double ecl = sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * T) * T) * T);
    transform<abs_cs, ort_cs, ecl_cs, sph_cs>::backward(pos, ecl, out);
}
