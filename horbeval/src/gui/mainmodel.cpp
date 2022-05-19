#include <mainmodel.h>
#include <logger.h>

#include <interface.h>

#include <forecast.h>

#include <assertion.h>
#include <conversion.h>
#include <statistics.h>

#include <fstream>
#include <iomanip>

constexpr double sec_per_day{ 86400 };

project_settings gl_settings;

time_h to_time(const QDateTime& dt);

QDateTime from_time(time_h t) 
{
    return QDateTime::fromMSecsSinceEpoch(t.mcs / 1000);
}

std::ostream& operator<<(std::ostream& out, const motion_params& mp);
std::ostream& operator<<(std::ostream& out, const rotational_params& rp);
std::ostream& operator<<(std::ostream& out, const round_plane_info& p);

template<typename F>
F open_file(const std::string& filename, const std::string& errormsg)
{
    F fs{ filename };
    ASSERT(fs.is_open(), errormsg);
    return fs;
}

std::ifstream open_infile(const QString& filepath)
{
    auto filename = filepath.toStdString();
    return open_file<std::ifstream>(filename, "Не удалось открыть для чтения файл " + filename);
}

std::ofstream open_outfile(const QString& filepath) 
{
    auto filename = filepath.toStdString();
    return open_file<std::ofstream>(filename, "Не удалось открыть для записи файл " + filename);
}

void mainmodel::read_geopotential(const QString& filename)
{
    EGM96::load_harmonics(open_infile(filename));
}

void mainmodel::read_tle(const QString& filename)
{
    _orbprovider = std::make_unique<orbit_observation_provider>(open_infile(filename));
}

void mainmodel::read_measurements(const QString& mfilename, const QString& ofilename)
{
    _rotprovider = std::make_unique<rotation_observation_provider>(
        open_infile(ofilename), open_infile(mfilename)
    );
}

void mainmodel::set_interval(const QDateTime& tn, const QDateTime& tk)
{
    _tn = to_time(tn);
    _tk = to_time(tk);
}

constexpr inline size_t min_required_count{ 3 };
#define check_distance(beg, end, msg) ASSERT(std::distance(beg, end) >= min_required_count, msg);

residuals_info compute_residuals(const forecast<6>& f, measurement_iter beg, measurement_iter end)
{
    residuals_info res;
    res.array.resize(std::distance(beg, end));
    for (size_t i{}; beg != end; ++beg, ++i) {
        auto mp = f.params(beg->t);
        double tmp{};
        for (size_t k{}; k < 3; ++k) {
            tmp += sqr(mp.v[k] - beg->v[k]);
        }
        res.array[i] = std::sqrt(tmp);
    }
    auto stat = mean_std(std::begin(res.array), std::end(res.array));
    res.mean = stat.mean;
    res.std = stat.std;
    return res;
}

QDateTime mainmodel::tn() const {
    return from_time(_orbprovider->begin()->t);
}

QDateTime mainmodel::tk() const {
    return from_time((--_orbprovider->end())->t).addMSecs(1);
}

void mainmodel::compute(computation* const info, const QString& filename) const
{
    try {
        file_logger log;
        if (!filename.isEmpty()) log = file_logger(filename);
        info->reset();

        auto [beg_orb, end_orb] = _orbprovider->retrieve(_tn, _tk);
        check_distance(
            beg_orb, end_orb, 
            format(
                "Недостаточное кол-во орбитальных измерений в мерном интервале (минимальное кол-во %",
                min_required_count
            )
        )

        auto tk = (end_orb - 1)->t;

        info->interval.begin = beg_orb;
        info->interval.end = end_orb - 1;
        info->interval_info_updated();

        auto [beg_rot, end_rot] = _rotprovider->retrieve(_tn, _tk);
        check_distance(
            beg_rot, end_rot, 
            format(
                "Недостаточное кол-во измерений блеска в мерном интервале (минимальное кол-во %",
                min_required_count
            )
        )

        // исходные параметры движения
        motion_params mp{};
        std::copy(beg_orb->v, beg_orb->v + 6, mp.v.data());
        mp.t = beg_orb->t;

        // пишем орбитальные измерения
        log << "Массив орбитальных измерений" << std::endl;
        size_t count{};
        for (auto it = beg_orb; it != end_orb; ++it) {
            log << std::setw(6) << ++count << ' ' << *it << std::endl;
        }

        info->init_motion.v = mp.v;
        info->init_motion.r = compute_residuals(make_forecast<basic_motion_model>(mp, tk), beg_orb, end_orb);
        info->init_motion_info_updated();
        
        log << std::endl;
        log << "Исходные параметры движения" << std::endl << mp << std::endl;
        log << "Невязки" << std::endl;
        for (size_t i{}; i < info->init_motion.r.array.size(); ++i) {
            log << std::setw(6) << i + 1 << ' ' << (beg_orb + i)->t << ' ' << info->init_motion.r.array[i] << std::endl;
        }
        log << std::endl;

        basic_linear_interface bi{ mp, beg_orb, end_orb };
        size_t iterations = newton(bi, log.rdbuf());
        mp = bi.parameters();

        info->prev_object.rad = gl_settings.object.rad;
        info->prev_motion.v = mp.v;
        info->prev_motion.r = compute_residuals(make_forecast(mp, tk), beg_orb, end_orb);
        info->prev_motion_info_updated();

        log << std::endl;
        log << "Кол-во итераций = " << iterations << std::endl;
        log << "Уточненные параметры движения" << std::endl << mp << std::endl;
        log << "Невязки" << std::endl;
        for (size_t i{}; i < info->prev_motion.r.array.size(); ++i) {
            log << std::setw(6) << i + 1 << ' ' << (beg_orb + i)->t << ' ' << info->prev_motion.r.array[i] << std::endl;
        }
        log << std::endl;

        rotational_params rp = estimate_rotation(mp, beg_rot, end_rot, { 0, 0, 1 }, log.rdbuf());
        info->rotation.axis = rp.axis;
        info->rotation.vel = rp.vel;
        info->rotation_info_updated();

        log << std::endl;
        log << "Исходные параметры КА" << std::endl << gl_settings.object << std::endl << std::endl;
        
        extended_linear_interface ei{ mp, rp, gl_settings.object, beg_orb, end_orb };
        iterations = newton(ei, log.rdbuf());
        mp = ei.parameters();
        auto plane = ei.object_params();

        info->next_object.rad = plane.rad;
        info->next_motion.v = mp.v;
        info->next_motion.r = compute_residuals(make_forecast(mp, tk, rp, plane), beg_orb, end_orb);
        info->next_motion_info_updated();

        log << std::endl;
        log << "Кол-во итераций = " << iterations << std::endl;
        log << "Уточненные параметры движения" << std::endl << mp << std::endl;
        log << "Уточнённые параметры КА" << std::endl << plane << std::endl << std::endl;
        log << "Невязки" << std::endl;
        for (size_t i{}; i < info->next_motion.r.array.size(); ++i) {
            log << std::setw(6) << i + 1 << ' ' << (beg_orb + i)->t << ' ' << info->next_motion.r.array[i] << std::endl;
        }

    } catch (const std::exception& error) {
        throw std::runtime_error(format("Во время расчёта произошла ошибка! %", error.what()));
    }
}

template<size_t size>
std::ostream& operator<<(std::ostream& out, const vec<size>& v)
{
    static_assert(size <= 6, "для размерности вектора больше 6 функция не реализована");

    const char* descs[6]{
        "X (м) = ", "Y (м) = ", "Z (м) = ",
        "Vx (м/с) = ", "Vy (м/с) = ", "Vz (м/с) = "
    };
    for (size_t i{}; i < v.size(); ++i) out << descs[i] << v[i] << std::endl; 
    return out;    
}

std::ostream& operator<<(std::ostream& out, const motion_params& mp)
{
    return out << "T = " << mp.t << std::endl << mp.v;
}

std::ostream& operator<<(std::ostream& out, const round_plane_info& p)
{
    return out << "масса = " << p.mass << " кг радиус = " << p.rad << " м к-т отражения = " << p.refl;
}

std::ostream& operator<<(std::ostream& out, const residuals_info& p)
{
    return out << "cр.знач. = " << p.mean << ' ' << "СКО = " << p.std << std::endl;
}

std::ostream& operator<<(std::ostream& out, const motion_info& p)
{
    out << std::setprecision(3);
    return out <<
        "вектор состояния" << std::endl << p.v << 
        "невязки: " << std::endl << p.r;
}

void computation::reset()
{
    _stage = computation_stage::zero;
    std::fill(std::begin(init_motion.r.array), std::end(init_motion.r.array), 0);
    std::fill(std::begin(prev_motion.r.array), std::end(prev_motion.r.array), 0);
    std::fill(std::begin(next_motion.r.array), std::end(next_motion.r.array), 0);
}

std::string computation::to_string() const 
{
    std::stringstream out;
    out.setf(std::ios::fixed);
    out << std::setprecision(4);

    if (_stage >= computation_stage::first) {
        out << 
            "Мерный интервал" << std::endl <<
            "нач. время " << interval.begin->t << std::endl <<
            "длит-ть (сут) = " << (interval.end->t - interval.begin->t) / sec_per_day << std::endl;
        out << std::endl;
    }
    if (_stage >= computation_stage::fourth) {
        out << "Параметры вращения:\n" << "ось в АСК ";
        for (size_t i{}; i < 3; ++i) out << rotation.axis[i] << ' ';
        out << std::endl << "T вращ. (сут) = " << (360 / (rad_to_deg(rotation.vel) * sec_per_day)) << std::endl;
        out << std::endl;
    }
    if (_stage >= computation_stage::second) {
        out << "Исходные параметры движения\n" << init_motion;
        out << std::endl;
    }
    if (_stage >= computation_stage::third) {
        out << "Уточнённые параметры движения\n\n";
        out << "без учёта вращения:\n" << prev_motion << "радиус (м) = " << prev_object.rad << std::endl; 
        out << std::endl;
    }
    if (_stage >= computation_stage::fifth) {
        out << "с учётом вращения:\n" << next_motion << "радиус (м) = " << next_object.rad << std::endl;
    }
    return out.str();
}

project_settings read(const QString& filename)
{
    project_settings read(std::istream&& in);
    return read(open_infile(filename));
}

void write(const QString& filename, const project_settings& p)
{
    void write(std::ostream&& out, const project_settings& p);
    write(open_outfile(filename), p);
}