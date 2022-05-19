#include <ball.h>
#include <transform.h>
#include <conversion.h>

double jc2000(time_h t);

void solar_model::coordinates(time_h t,  double* const ort, double* const sph)
{
    const double jc = jc2000(t);
    // solar average longitude
    double L = sec_to_rad(1009677.85 + (100 * SEC_PER_ROUND + 2771.27 + 1.089 * jc) * jc);
    // solar perigee average longitude
    double lc = sec_to_rad(1018578.046 + (6190.046 + (1.666 + 0.012 * jc) * jc) * jc);
    // the Earth's orbit eccentricity
    double e = 0.0167086342 - (0.000004203654 + (0.00000012673 + 0.00000000014 * jc) * jc) * jc;
    // average ecliptic inclination
    double ecl = sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * jc) * jc) * jc);
    // ecliptic average longitude of lunar ascending node
    double omega = sec_to_rad(450160.280 - (5 * SEC_PER_ROUND + 482890.539 - (7.455 + 0.008 * jc) * jc) * jc);
    // long periodic nutation of the Earth
    double psi = sec_to_rad(-17.1996 * std::sin(omega));
    // solar longitude
    double longitude = L + 2 * e * std::sin(L - lc) + 1.25 * e * e * std::sin(2 * (L - lc));
    double sinl{ std::sin(longitude) };
    double cosl{ std::cos(longitude) };
    double sine{ std::sin(ecl) };
    double cose{ std::cos(ecl) };
    double pos[3];
    pos[1] = std::atan(sinl * sine / std::sqrt(cosl * cosl + sinl * sinl * cose * cose));
    pos[2] = std::atan(sinl / cosl * cose);
    if (pos[1] * pos[2] < 0) pos[2] += PI;
    // mean distance from the sun to the earth
    constexpr double ac{ 1.4959787e11 };
    double cosllc = std::cos(L - lc);
    pos[0] = ac * (1 - e * (cosllc - e * 0.25 * (1 - cosllc)));
    // a constant of the aberration
    constexpr double hi{ sec_to_rad(20.49552) };
    // corrections caused by nutation
    pos[2] += 0.061165 * psi - hi;
    pos[1] += hi * sine * cosl;
    
    if (sph) std::copy(pos, pos + 3, sph);
    if (ort) transform<abs_cs, sph_cs, abs_cs, ort_cs>::forward(pos, ort);
}

void lunar_model::coordinates(time_h t, double* const out)
{
    const double jc = jc2000(t);
    // radius of Earth's equator
    double r{ 6378136 };
    // mean lunar anomaly
    double la = sec_to_rad((485866.733 + (1325 * SEC_PER_ROUND + 715922.633 + (31.31 + 0.064 * jc) * jc) * jc));
    // mean solar anomaly
    double sa = sec_to_rad((1287099.804 + (99 * SEC_PER_ROUND + 1292581.224 - (0.577 + 0.012 * jc) * jc) * jc));
    // average arg of lunar latitude
    double f = sec_to_rad((335778.877 + (1342 * SEC_PER_ROUND + 295263.137 - (13.257 - 0.011 * jc) * jc) * jc));
    // average alongation (a difference between solar and lunar longitudes)
    double d = sec_to_rad((1072261.307 + (1236 * SEC_PER_ROUND + 1105601.328 - (6.891 - 0.019 * jc) * jc) * jc));
    // lunar coordinates
    auto pos = out;;
    // lunar ecliptic latitude
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
        15.56 * std::sin(2 * (la - d) + f)
    );
    // lunar ecliptic longitude
    pos[2] = sec_to_rad(
    // lunar mean longitude
    785939.157 + (1336 * SEC_PER_ROUND + 1108372.598 + (5.802 + 0.019 * jc) * jc) * jc +
    // delta lunar longitude
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
        18.02 * std::sin(sa - d)
    );
    pos[2] = fit_to_round(pos[2]);
    // lunar paralax
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
        0.601 * std::cos(la - 4 * d)
    );
    pos[0] = r / paralax;
    // average ecliptic inclination
    double ecl = sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * jc) * jc) * jc);

    double buf[3];
    //sph_to_ort(pos, buf);
    //ECS_to_ACS(buf, ecl, pos);
}

void solar_coordinates(time_h t, double* const out)
{
    double buf[3];  
    solar_model::coordinates(t, buf);
    transform<abs_cs, sph_cs, grw_cs, ort_cs>::forward(buf, sidereal_time_mean(t), out);
}

void lunar_coordinates(time_h t, double* const out)
{
    double buf[3];
    lunar_model::coordinates(t, buf);
    transform<abs_cs, sph_cs, grw_cs, ort_cs>::forward(buf, sidereal_time_mean(t), out);
}
