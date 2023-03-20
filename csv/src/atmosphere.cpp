#include <atmosphere.hpp>
#include <formatting.hpp>
#include <timeutility.hpp>
#include <cmath>

double atmosphere1981(double h)
{
    constexpr double height[9]{0.0, 0.2e+5, 0.6e+5,
                               1.0e+5, 1.5e+5, 3.0e+5,
                               6.0e+5, 9.0e+5, 10e6};
    constexpr double a0[9]{0.12522, 0.91907e-2, 0.31655e-4,
                           0.54733e-7, 0.20474e-9, 0.19019e-11,
                           0.11495e-13, 0.58038e-15, 1e-17};
    constexpr double k1[9]{-0.20452e-8, 0.62669e-9, -0.86999e-9,
                           0.12870e-8, 0.10167e-9, 0.97266e-11,
                           0.15127e-10, 0.0, 0};
    constexpr double k2[9]{0.90764e-4, 0.16739e-3, 0.12378e-3,
                           0.17527e-3, 0.45825e-4, 0.19885e-4,
                           0.14474e-4, 0.39247e-5, 1e-6};
    std::size_t index{};
    while (h > height[index])
    {
        ++index;
    }
    h -= height[--index];
    return a0[index] * std::exp(h * (h * k1[index] - k2[index]));
}

double atmosphere2004_static(double h)
{
    constexpr double height[4]{0, 20, 60, 100};
    constexpr double coef[4][3]{
        {1.228, -9.0764e-2, -2.0452e-3},
        {9.013e-2, -0.16739, 6.2669e-4},
        {3.104e-4, -0.137, -7.8653e-4},
        {3.66e-7, -0.18553, 1.5397e-3}};
    std::size_t index{1};
    for (; index < 4; ++index)
    {
        if (h < height[index])
            break;
    }
    h -= height[--index];
    return coef[index][0] * (std::exp(h * (coef[index][1] + h * coef[index][2])));
}

enum struct coefficient
{
    a,
    b,
    c,
    d,
    e,
    et,
    l,
    n,
    phi,
    A
};

template <std::size_t isa_size, std::size_t coef_size>
constexpr auto &get_coefficients(double h, double const (&heights)[isa_size], std::size_t isa,
                                 double const (&greater)[isa_size][coef_size], double const (&less)[isa_size][coef_size])
{
    return h < heights[isa] ? less[isa] : greater[isa];
}

template <coefficient>
class coefficient_data;

template <>
struct coefficient_data<coefficient::a>
{
    static constexpr double greater[7][7]{
        {17.8781, -0.132025, 0.000227717, -2.2543e-7, 1.33574e-10, -4.50458e-14, 6.72086e-18},
        {-2.54909, 0.0140064, -0.00016946, 3.27196e-7, -2.8763e-10, 1.22625e-13, -2.05736e-17},
        {-13.9599, 0.0844951, -0.000328875, 5.05918e-7, -3.92299e-10, 1.52279e-13, -2.35576e-17},
        {-23.3079, 0.135141, -0.000420802, 5.73717e-7, -4.03238e-10, 1.42846e-13, -2.01726e-17},
        {-14.7264, 0.0713256, -0.000228015, 2.8487e-7, -1.74383e-10, 5.08071e-14, -5.34955e-18},
        {-4.912, 0.0108326, -8.10546e-5, 1.15712e-7, -8.13296e-11, 3.04913e-14, -4.94989e-18},
        {-5.40952, 0.00550749, -3.78851e-5, 2.4808e-8, 4.92183e-12, -8.65011e-15, 1.9849e-18},
    };
    static constexpr double less[7][7]{
        {26.8629, -0.451674, 0.00290397, -1.06953e-05, 2.21598e-08, -2.42941e-11, 1.09926e-14},
        {27.4598, -0.463668, 0.002974, -1.0753e-05, 2.17059e-08, -2.30249e-11, 1.00123e-14},
        {28.6395, -0.490987, 0.00320649, -1.1681e-05, 2.36847e-08, -2.51809e-11, 1.09536e-14},
        {29.6418, -0.514957, 0.00341926, -1.25785e-05, 2.5727e-08, -2.75874e-11, 1.21091e-14},
        {30.1671, -0.527837, 0.00353211, -1.30227e-05, 2.66455e-08, -2.85432e-11, 1.25009e-14},
        {29.7578, -0.517915, 0.00342699, -1.24137e-05, 2.48209e-08, -2.58413e-11, 1.09383e-14},
        {30.7854, -0.545695, 0.00370328, -1.37072e-05, 2.80614e-08, -3.00184e-11, 1.31142e-14},
    };
    static constexpr double heights[7]{500, 500, 500, 500, 500, 500, 500};

public:
    static constexpr auto &get(double h, std::size_t isa)
    {
        return get_coefficients(h, heights, isa, greater, less);
    }
};

template <>
class coefficient_data<coefficient::b>
{
    static constexpr double greater[7][5]{
        {23.1584, -0.0802147, 0.000105824, -6.15036e-08, 1.32453e-11},
        {33.2732, -0.111099, 0.000141421, -7.94952e-08, 1.65836e-11},
        {39.1961, -0.12352, 0.000149015, -7.9705e-08, 1.58772e-11},
        {43.2469, -0.126973, 0.000142637, -7.09985e-08, 1.31646e-11},
        {49.5738, -0.138613, 0.000147851, -6.96361e-08, 1.21595e-11},
        {11.278, 0.00143478, -3.69846e-05, 3.58318e-08, -9.91225e-12},
        {-52.6184, 0.214689, -0.000294882, 1.71171e-07, -3.60582e-11},
    };
    static constexpr double less[7][5]{
        {0.0687894, -0.00284077, 1.83922e-05, 9.19605e-09, -4.16873e-11},
        {0.15073, -0.00400889, 2.43937e-05, -9.92772e-09, -1.82239e-11},
        {0.0479451, -0.00239453, 1.70335e-05, -1.31626e-09, -1.74032e-11},
        {0.0223448, -0.0019798, 1.54101e-05, -2.3543e-09, -1.24994e-11},
        {-0.00326391, -0.00159869, 1.40443e-05, -3.02287e-09, -9.2016e-12},
        {-0.0514749, -0.000921059, 1.15147e-05, -1.22901e-09, -8.13104e-12},
        {-0.107255, -0.000174343, 9.02759e-06, -3.16512e-10, -6.14e-12},
    };
    static constexpr double heights[7]{00, 660, 760, 800, 860, 900, 1000};

public:
    static constexpr auto &get(double h, std::size_t isa)
    {
        return get_coefficients(h, heights, isa, greater, less);
    }
};

template <>
class coefficient_data<coefficient::c>
{
    static constexpr double greater[7][5]{
        {50.5034, -0.170541, 0.000217232, -1.21902e-07, 2.54037e-11},
        {61.624, -0.192967, 0.000228061, -1.18715e-07, 2.29638e-11},
        {53.2623, -0.144342, 0.00014659, -6.46443e-08, 1.04227e-11},
        {18.2236, -0.00840024, -3.88e-05, 4.31384e-08, -1.23832e-11},
        {-31.8442, 0.168327, -0.000262603, 1.65454e-07, -3.69355e-11},
        {-48.7208, 0.222996, -0.000321884, 1.91495e-07, -4.08067e-11},
        {-147.859, 0.531652, -67.1937, 3.64787e-07, -7.26268e-11},
    };
    static constexpr double less[7][5]{
        {-1.04825, 0.0166305, -9.24263e-05, 2.72382e-07, -2.41355e-10},
        {-0.93106, 0.0141537, -7.29862e-05, 2.00294e-07, -1.62006e-10},
        {-0.820867, 0.0119916, -5.79835e-05, 1.50707e-07, -1.13026e-10},
        {-0.744047, 0.0104743, -4.78544e-05, 1.18513e-07, -8.31498e-11},
        {-0.722471, 0.00980317, -4.25245e-05, 9.95544e-08, -6.55175e-11},
        {-0.687482, 0.00916594, -3.80932e-05, 8.51275e-08, -5.29972e-11},
        {-0.739984, 0.00952854, -3.62727e-05, 7.3887e-08, -4.23907e-11},
    };
    static constexpr double heights[7]{640, 700, 760, 820, 860, 920, 980};

public:
    static constexpr auto &get(double h, std::size_t isa)
    {
        return get_coefficients(h, heights, isa, greater, less);
    }
};

template <>
class coefficient_data<coefficient::d>
{
    static constexpr double values[7][5]{
        {-0.351899, 0.00577056, 9.95819e-07, -7.25324e-09, 2.9759e-12},
        {-0.047813, 0.00380813, 4.22771e-06, -8.66826e-09, 3.06712e-12},
        {0.20981, 0.00262881, 4.24379e-06, -6.67328e-09, 2.13496e-12},
        {0.265174, 0.00275836, 2.08668e-06, -3.69543e-09, 1.11862e-12},
        {0.23047, 0.00338331, -5.52305e-07, -8.23607e-10, 2.21349e-13},
        {0.170074, 0.00406131, -2.82114e-06, 1.38369e-09, -4.27908e-13},
        {0.088141, 0.00468253, -4.24609e-06, 2.53509e-09, -7.29031e-13},
    };

public:
    static constexpr auto &get(std::size_t isa)
    {
        return values[isa];
    }
};

template <>
class coefficient_data<coefficient::e>
{
    static constexpr double greater[7][9]{
        {38.6199, -0.132147, 0.000175411, -1.02417e-07, 2.21446e-11, -0.2067, 0.097533, -0.011817, 0.0016145},
        {51.249, -0.167373, 0.000211832, -1.18221e-07, 2.45055e-11, -0.16971, 0.07983, -0.0094393, 0.0012622},
        {68.4746, -0.215659, 0.000262273, -1.40972e-07, 2.82285e-11, -0.14671, 0.068808, -0.0079836, 0.0010535},
        {58.422, -0.166664, 0.000185486, -9.12345e-08, 1.67118e-11, -0.1315, 0.061603, -0.0070866, 0.00092813},
        {7.20188, 0.0216109, -6.52882e-05, 5.37077e-08, -1.4095e-11, -0.120916, 0.056538, -0.0064324, 0.00083723},
        {21.5948, -0.0202239, -1.72029e-05, 2.83017e-08, -8.94486e-12, -0.11363, 0.053178, -0.0060436, 0.00077982},
        {-88.4076, 0.338518, -0.000445581, 2.51729e-07, -5.203e-11, -0.10444, 0.048551, -0.0053567, 0.00068809},
    };
    static constexpr double less[7][9]{
        {-0.731596, 0.00597345, -5.82037e-06, 6.84634e-08, -9.50483e-11, -0.2067, 0.097533, -0.011817, 0.0016145},
        {-0.752175, 0.00565925, 1.8082e-06, 3.33822e-08, -5.13965e-11, -0.16971, 0.07983, -0.0094393, 0.0012622},
        {-0.570476, 0.00295802, 1.68896e-05, -4.7475e-09, -1.72711e-11, -0.14671, 0.068808, -0.0079836, 0.0010535},
        {-0.949573, 0.00813121, -3.87813e-06, 2.37694e-08, -2.77469e-11, -0.1315, 0.061603, -0.0070866, 0.00092813},
        {-0.967598, 0.00841991, -3.585e-06, 1.74801e-08, -1.96221e-11, -0.120916, 0.056538, -0.0064324, 0.00083723},
        {-1.02278, 0.00923633, -6.10128e-06, 1.78211e-08, -1.70073e-11, -0.11363, 0.053178, -0.0060436, 0.00077982},
        {-0.757903, 0.00606068, 7.85296e-06, -9.74891e-09, 1.58377e-12, -0.10444, 0.048551, -0.0053567, 0.00068809},
    };
    static constexpr double heights[7]{600, 700, 780, 800, 800, 900, 760};

public:
    static constexpr auto &get(double h, std::size_t isa)
    {
        return get_coefficients(h, heights, isa, greater, less);
    }
};

template <>
class coefficient_data<coefficient::et>
{
    static constexpr double values[7][4]{
        {-0.2061, 0.094449, -0.0087953, 0.00088385},
        {-0.169279, 0.077599, -0.0071375, 0.00069025},
        {-0.146377, 0.067052, -0.0060951, 0.00057456},
        {-0.13121, 0.060105, -0.0054388, 0.00050585},
        {-0.12067, 0.055232, -0.004958, 0.00045512},
        {-0.113399, 0.051994, -0.0046876, 0.00042548},
        {-0.104243, 0.047573, -0.0041711, 0.00037068},
    };

public:
    static constexpr auto &get(std::size_t isa)
    {
        return values[isa];
    }
};

template <>
class coefficient_data<coefficient::l>
{
    static constexpr double greater[7][5]{
        {48.6536, -0.170291, 0.000226242, -1.32032e-07, 2.85193e-11},
        {54.4867, -0.178298, 0.000222725, -1.227e-07, 2.51316e-11},
        {60.1267, -0.183144, 0.000212481, -1.08497e-07, 2.0571e-11},
        {47.0996, -0.12526, 0.000126352, -5.51584e-08, 8.75272e-12},
        {50.6174, -0.129047, 0.000124842, -5.24993e-08, 8.08272e-12},
        {8.01942, 0.0185302, -6.14733e-05, 4.97674e-08, -1.26162e-11},
        {-15.5728, 0.0936704, -0.000149036, 9.42151e-08, -2.0961e-11},
    };
    static constexpr double less[7][5]{
        {-0.407768, 0.00148506, 1.25357e-05, 3.77311e-08, -7.78953e-11},
        {-0.902739, 0.00826803, -1.25448e-05, 6.12853e-08, -7.07966e-11},
        {-0.733037, 0.00523396, 6.35667e-06, 1.09065e-08, -2.61427e-11},
        {-1.31444, 0.0133124, -2.55585e-05, 5.43981e-08, -4.33784e-11},
        {-1.20026, 0.0114087, -1.47324e-05, 2.7804e-08, -2.2632e-11},
        {-1.52158, 0.015704, -3.02859e-05, 4.57668e-08, -2.82926e-11},
        {-1.67664, 0.0177194, -3.69498e-05, 5.09134e-08, -2.82878e-11},
    };
    static constexpr double heights[7]{640, 660, 740, 800, 860, 900, 900};

public:
    static constexpr auto &get(double h, std::size_t isa)
    {
        return get_coefficients(h, heights, isa, greater, less);
    }
};

template <>
class coefficient_data<coefficient::n>
{
    static constexpr double values[3]{2.058, 5.887e-3, -4.012e-6};

public:
    static constexpr auto &get()
    {
        return values;
    }
};

template <>
class coefficient_data<coefficient::phi>
{
    static constexpr double values[7]{0.5411, 0.5515, 0.5585, 0.5585, 0.5585, 0.5585, 0.5585};

public:
    static constexpr auto &get(std::size_t isa)
    {
        return values[isa];
    }
};

template <>
class coefficient_data<coefficient::A>
{
    static constexpr double values[9]{-2.53418e-2, -2.44075e-3, 3.08389e-6, 2.90115e-6, -4.99606e-8, 3.36327e-10, -1.0966e-13, 1.73227e-15, -1.06271e-18};
    ;

public:
    static constexpr auto &get()
    {
        return values;
    }
};

auto isa_index(double f81)
{
    constexpr double isa_thresholds[7]{75, 100, 125, 150, 175, 200, 250};
    std::size_t index{};
    for (; index < 5; ++index)
    {
        if (f81 < isa_thresholds[index])
            break;
    }
    --index;
    return std::make_pair(index, isa_thresholds[index]);
}

template <std::size_t size>
double compute_polynomial(double h, double const (&arr)[size])
{
    double res{};
    for (std::size_t i{size - 1}; i > 0; --i)
    {
        res += arr[i];
        res *= h;
    }
    return res + arr[0];
}

double radius(double const *v)
{
    double rad{};
    for (std::size_t i{}; i < 3; ++i)
    {
        rad += v[i] * v[i];
    }
    return std::sqrt(rad);
}

double atmosphere2004_dynamic(double const *pos, double h,
                              int day, double lg, double incl,
                              double f10_7, double f81, double kp)
{
    auto [index, f0] = isa_index(f81);

    auto &l = coefficient_data<coefficient::l>::get(h, index);
    double k0 = 1 + compute_polynomial(h, l) * (f81 - f0) / f0;

    auto &c = coefficient_data<coefficient::c>::get(h, index);
    auto &n = coefficient_data<coefficient::n>::get();
    double beta = lg + coefficient_data<coefficient::phi>::get(index);
    double rad = radius(pos);
    double cosphi = 1 / rad * (pos[2] * std::sin(incl) + std::cos(incl) * (pos[0] * std::cos(beta) + pos[1] * std::sin(beta)));
    cosphi = std::sqrt(0.5 * (1 + cosphi));
    double k1 = compute_polynomial(h, c) * std::pow(cosphi, compute_polynomial(h, n));

    auto &d = coefficient_data<coefficient::d>::get(index);
    auto &A = coefficient_data<coefficient::A>::get();
    double k2 = compute_polynomial(day, A) * compute_polynomial(h, d);

    auto &b = coefficient_data<coefficient::b>::get(h, index);
    double df = f10_7 - f81;
    double k3 = compute_polynomial(h, b) * df / (f81 + std::abs(df));

    auto &e = coefficient_data<coefficient::e>::get(h, index);
    using e5_t = double const(&)[5];
    using e4_t = double const(&)[4];
    double k4 = compute_polynomial(h, reinterpret_cast<e5_t>(e)) * compute_polynomial(kp, reinterpret_cast<e4_t>(e[5]));

    auto &a = coefficient_data<coefficient::a>::get(h, index);
    double rho = 1.58868e-8 * std::exp(compute_polynomial(h, a));
    return rho * k0 * (1 + k1 + k2 + k3 + k4);
}

int day_of_the_year(time_t t);

double atmosphere2004(double const *p, double h, time_t t, double sol_long, double sol_incl,
                      double f10_7, double f81, double kp)
{
    h *= 1e-3;
    if (h > 1500)
    {
        return 0;
    }
    if (h < 120)
    {
        return atmosphere2004_static(h);
    }
    return atmosphere2004_dynamic(p, h, day_of_the_year(t), sol_long, sol_incl, f10_7, f81, kp);
}
