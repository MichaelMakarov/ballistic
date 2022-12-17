#include <formatting.hpp>
#include <timefmt.hpp>
#include <transform.hpp>
#include <fstream>
#include <functional>
#include <iomanip>

//----------------------------------------------//
//      открытие файлов на чтение и запись      //
//----------------------------------------------//

template <typename>
struct open_file_error;

template <>
struct open_file_error<std::ofstream>
{
    static std::string message(const char *filename)
    {
        using namespace std::string_literals;
        return "Не удалось открыть для записи файл "s + filename;
    }
};
template <>
struct open_file_error<std::ifstream>
{
    static std::string message(const char *filename)
    {
        using namespace std::string_literals;
        return "Не удалось открыть для чтения файл "s + filename;
    }
};

template <typename F>
F open_file(const char *filename)
{
    F file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error(open_file_error<F>::message(filename));
    }
    return file;
}

std::ifstream open_infile(std::string_view filename)
{
    return open_file<std::ifstream>(filename.data());
}

std::ofstream open_outfile(std::string_view filename)
{
    return open_file<std::ofstream>(filename.data());
}

//------------------------------------------//
//          форматирование структур         //
//------------------------------------------//

/**
 * @brief Знак градуса
 *
 */
constexpr char degree[3]{char(0xC2), char(0xB0)};
/**
 * @brief Знак фи
 *
 */
constexpr char phi[3]{char(0xCF), char(0x95)};
/**
 * @brief Знак лямбда
 *
 */
constexpr char lambda[3]{char(0xCE), char(0xBB)};
/**
 * @brief Знак дельта
 *
 */
constexpr char delta[3]{char(0xCE), char(0x94)};
/**
 * @brief Знак омега
 *
 */
constexpr char omega[3]{char(0xCF), char(0x89)};
/**
 * @brief Знак епсилон
 *
 */
constexpr char epsilon[3]{char(0xCE), char(0xB5)};
/**
 * @brief Метр
 *
 */
constexpr const char *meter{"(м)"};
/**
 * @brief Метр в сек
 *
 */
constexpr const char *mps{"(м/с)"};
/**
 * @brief Метр в квадрате
 *
 */
constexpr auto meter_sq{"(м^2)"};
/**
 * @brief Равенство
 *
 */
constexpr auto equal{" = "};
/**
 * @brief параметры движения
 *
 */
constexpr const char *coordinates[]{"x", "y", "z", "vx", "vy", "vz", "s", epsilon};
/**
 * @brief Размерности координат
 *
 */
constexpr const char *dimensions[]{meter, meter, meter, mps, mps, mps, meter_sq, ""};

double sec_to_days(double sec)
{
    return sec * (1. / 86400);
}

constexpr char separator(bool flag)
{
    return flag ? ' ' : '\n';
}

template <size_t size>
void print_vec(std::ostream &os, double const v[size], char sep)
{
    for (size_t i{}; i < size; ++i)
        os << coordinates[i] << equal << v[i] << dimensions[i] << sep;
}

template <size_t size>
void print_vec(std::ostream &os, vec<size> const &v, char sep)
{
    print_vec<size>(os, v.data(), sep);
}

template <size_t size>
void print_cor(std::ostream &os, vec<size> const &v)
{
    for (size_t i{}; i < v.size(); ++i)
        os << delta << coordinates[i] << equal << v[i] << dimensions[i] << ' ';
}

template <size_t size>
void print_mat(std::ostream &os, vec<size> const &v)
{
    for (size_t i{}; i < v.size(); ++i)
        os << delta << "r/" << delta << coordinates[i] << equal << v[i] << " 1/" << dimensions[i] << ' ';
}

template <size_t _size>
void print_iteration(std::ostream &os, const optimization_iteration<_size> &i)
{
    auto prec = os.precision();
    os << "Итерация № " << i.n << std::endl;
    os << "Функция невязок r = " << std::setprecision(6) << i.r << std::setprecision(prec) << std::endl;
    os << "Вектор параметров ";
    print_vec(os, i.v, separator(true));
    os << std::endl;
    os << "Коррект. вектор ";
    print_cor(os, i.dv);
    os << std::endl;
    os << "Градиент ";
    print_mat(os, i.m);
}

template <size_t size>
void print_log(std::ostream &os, basic_logger<size> const &l)
{
    os << "Всего " << l.size() << " итераций." << std::endl;
    for (auto &i : l)
    {
        print_iteration(os, i);
        os << std::endl;
    }
}

void print_basic(std::ostream &os, const basic_info &m, bool full)
{
    os << "Уточненные параметры движения\n";
    print_vec(os, m.v, separator(full));
    os << std::endl;
    if (full)
    {
        os << "Оптимизация параметров \n";
        print_log(os, m.l);
        os << std::endl;
    }
    else
    {
        if (!m.l.empty())
            os << "Функция невязок = " << m.l.back().r << separator(full);
        else
            os << "Невязка не рассчитана из-за ошибки\n";
    }
}

void print_ext(std::ostream &os, const extended_info &m, bool full)
{
    char sep = separator(full);
    os << "Уточненные параметры движения\n";
    print_vec(os, m.v, sep);
    os << "s пов-ти = " << m.square << "(м^2)" << sep;
    os << epsilon << equal << m.refl << std::endl;
    if (full)
    {
        os << "Оптимизация параметров \n";
        print_log(os, m.l);
        os << std::endl;
    }
    else
    {
        if (!m.l.empty())
            os << "Функция невязок = " << m.l.back().r << separator(full);
        else
            os << "Невязка не рассчитана из-за ошибки\n";
    }
}

void print_tle(std::ostream &os, const orbit_data &d, bool full)
{
    auto sep = separator(full);
    std::format_to(std::ostream_iterator<char>{os}, "T = {} ", d.t);
    os << "\nПараметры движения" << sep;
    print_vec<6>(os, d.v, sep);
}

void print_interval(std::ostream &os, measuring_interval const &inter, bool full)
{
    std::format_to(std::ostream_iterator<char>{os}, "{} - {} кол-во измерений {}", inter.tn(), inter.tk(), inter.points_count());
    if (!full)
        return;
    auto [begin, end] = seance_iterators(inter);
    for (size_t i{1}; begin != end; ++begin)
    {
        os << "Сеанс № " << i << " обсерватория " << begin->id << ' ';
        print_vec<3>(os, begin->o, separator(full));
        os << " кол-во измерений " << begin->m.size() << std::endl;
        for (size_t j{}; j < begin->m.size(); ++j)
        {
            auto &meas = begin->m[j];
            std::format_to(std::ostream_iterator<char>{os}, "Измерение № {} T = {} ", j + 1, meas.t);
            os << lambda << equal << rad_to_deg(meas.i) << degree << ' ';
            os << phi << equal << rad_to_deg(meas.a) << degree << ' ';
            os << "зв.вел." << equal << meas.m << '\n';
        }
    }
}

void print_rot(std::ostream &os, rotation_info const &r, bool full)
{
    auto sep = separator(full);
    double buf[3]{};
    ::transform<abs_cs, sph_cs, abs_cs, ort_cs>::backward(r.r.axis.data(), buf);
    std::format_to(std::ostream_iterator<char>{os}, "Тн = {} ", r.r.tn);
    os << sep << "ось вращения " << sep;
    os << lambda << equal << rad_to_deg(buf[1]) << degree << sep;
    os << phi << equal << rad_to_deg(buf[2]) << degree << sep;
    auto precision = os.precision();
    os << omega << equal << std::setprecision(9) << sec_to_days(rad_to_deg(r.r.vel)) << degree << "/сут" << sep;
    os << std::setprecision(precision);
    if (full)
    {
        ::transform<abs_cs, sph_cs, abs_cs, ort_cs>::backward(r.n.data(), buf);
        os << sep << "нормаль к пов-ти " << sep;
        os << lambda << equal << rad_to_deg(buf[1]) << degree << sep;
        os << phi << equal << rad_to_deg(buf[2]) << degree << sep;
    }
}

void print_output(std::ostream &os, computational_output const &o, bool full)
{
    os << std::setprecision(3) << std::fixed;
    if (o.refer)
    {
        os << "Опорный ТЛЕ\n";
        print_tle(os, *o.refer, full);
        os << std::endl;
    }
    if (o.inter)
    {
        os << "Интервал измерений\n";
        print_interval(os, *o.inter, full);
        os << std::endl;
    }
    if (o.basic)
    {
        os << "Оптимизация без учета вращения\n";
        print_basic(os, *o.basic, full);
        os << std::endl;
    }
    if (o.rotation)
    {
        os << "Параметры вращения\n";
        print_rot(os, *o.rotation, full);
        os << std::endl;
    }
    if (o.extended)
    {
        os << "Оптимизация с учетом вращения\n";
        print_ext(os, *o.extended, full);
        os << std::endl;
    }
}
