#include <mainmodel.hpp>
#include <interval.hpp>
#include <settings.hpp>
#include <motion.hpp>

#include <fileutils.hpp>
#include <maths.hpp>
#include <ball.hpp>

#include <format>

using days_ratio_t = std::ratio_multiply<std::ratio<24>, std::ratio<3600>>;

bool cmp_to_be_earlier(const observation_seance &elem, time_type t)
{
    return elem.m.front().t < t;
}

bool cmp_to_be_later(time_type t, const observation_seance &elem)
{
    return t < elem.m.back().t;
}

class ballistic_computer
{
public:    
    /// @brief Массив данных считанных ТЛЕ
    std::vector<orbit_data> tles;    
    /// @brief Массив считанных сеансов измерений
    std::vector<observation_seance> seances;

public:
    std::unique_ptr<optimization_logger> compute(size_t tle_index, double days, std::size_t iter_count)
    {
        verify();
        // начальные условия из ТЛЕ
        orbit_data tle = tles.at(tle_index);
        // начальный момент времени
        auto tn = tle.t;
        // конечный момент времени
        auto tk = tn + std::chrono::duration_cast<time_type::duration>(std::chrono::duration<double, days_ratio_t>(days));
        // начальный сеанс на мерном инетрвале
        auto begin = std::lower_bound(std::begin(seances), std::end(seances), tn, &cmp_to_be_earlier);
        // конечный сеанс
        auto end = std::upper_bound(std::begin(seances), std::end(seances), tk, &cmp_to_be_later);
        // Интервал
        measuring_interval inter{begin, end};
        // общее количество измерений
        size_t number = inter.points_count();
        if (number <= 7)
        {
            throw std::runtime_error(std::format("Недостаточное кол-во измерений {} на интервале {} - {}.", number, tn, tk));
        }
        // для сохранения итераций
        auto saver = std::make_unique<optimization_logger>(iter_count, inter);
        run_optimization_s(inter, tle, *saver, iter_count);
        return saver;
    }

private:
    void verify() const
    {
        if (egm::harmonics.empty())
        {
            throw std::runtime_error("Гармоники геопотенциала Земли не загружены.");
        }
        if (tles.empty())
        {
            throw std::runtime_error("Данные TLE отсутствуют.");
        }
        if (seances.empty())
        {
            throw std::runtime_error("Данные измерений отсутствуют.");
        }
    }
};

computational_model::computational_model(QObject *parent) : QObject(parent),
                                                            _computer{std::make_unique<ballistic_computer>()}
{
}

computational_model::~computational_model() = default;

std::istream &operator>>(std::istream &is, potential_harmonic &p)
{
    return is >> p.cos >> p.sin;
}

void computational_model::read_gpt() const
{
    auto fin = open_infile(settings.gptpath);
    egm::read_harmonics(std::istream_iterator<potential_harmonic>{fin}, {});
}

std::vector<orbit_data> load_orbit_data(const std::string_view filename);

void computational_model::read_tle() const
{
    _computer->tles = load_orbit_data(settings.tlepath);
    emit tle_data_loaded();
}

std::vector<observation_seance> load_brightness_data(std::string_view obs_filename, std::string_view mes_filename);

void computational_model::read_measurements() const
{
    _computer->seances = load_brightness_data(settings.obspath, settings.mespath);
    emit measurement_data_loaded();
}

void computational_model::select_tle(int number)
{
    _index = number - 1;
}

void computational_model::select_interval(double interval)
{
    _interval = interval;
}

const orbit_data &computational_model::tle_by_index(int index) const
{
    return _computer->tles[static_cast<size_t>(index)];
}

const observation_seance &computational_model::seance_by_index(int index) const
{
    return _computer->seances[static_cast<size_t>(index)];
}

int computational_model::tle_count() const
{
    return static_cast<int>(_computer->tles.size());
}

int computational_model::seance_count() const
{
    return static_cast<int>(_computer->seances.size());
}

optimization_logger const *computational_model::get_logger() const
{
    return _logger.get();
}

void computational_model::compute(const std::string &filename)
{
    _logger = _computer->compute(static_cast<size_t>(_index), _interval, 20);
    if (!filename.empty())
    {
        auto fout = open_outfile(filename);
        _logger->print(fout);
    }
}
