#include <interval.hpp>
#include <logger.hpp>
#include <mainmodel.hpp>
#include <motion.hpp>

#include <ball.hpp>
#include <fileutils.hpp>
#include <maths.hpp>
#include <observation_utils.hpp>

#include <format>

namespace {
    using days_ratio_t = std::ratio_multiply<std::ratio<24>, std::ratio<3600>>;

    bool cmp_to_be_earlier(const observation_seance &elem, time_type t) {
        return elem.m.front().t < t;
    }

    bool cmp_to_be_later(time_type t, const observation_seance &elem) {
        return t < elem.m.back().t;
    }

} // namespace

class ballistic_computer {
  public:
    /// @brief Массив данных считанных ТЛЕ
    std::vector<orbit_data> tles;
    /// @brief Массив считанных сеансов измерений
    std::vector<observation_seance> seances;

  public:
    std::unique_ptr<optimization_logger> compute(size_t tle_index, double days, std::size_t iter_count) {
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
        if (number <= 7) {
            throw std::runtime_error(std::format("Недостаточное кол-во измерений {} на интервале {} - {}.", number, tn, tk));
        }
        // для сохранения итераций
        auto saver = std::make_unique<optimization_logger>(iter_count, inter);
        run_optimization(inter, tle, *saver, iter_count);
        return saver;
    }

  private:
    void verify() const {
        if (egm::harmonics.empty()) {
            throw std::runtime_error("Гармоники геопотенциала Земли не загружены.");
        }
        if (tles.empty()) {
            throw std::runtime_error("Данные TLE отсутствуют.");
        }
        if (seances.empty()) {
            throw std::runtime_error("Данные измерений отсутствуют.");
        }
    }
};

class table_data_provider_impl : public table_data_provider {
    static std::string headers[7];

  public:
    table_data_provider_impl(ballistic_computer const *computer)
        : _computer{computer} {
    }

    std::size_t get_rows_count() const override {
        return _computer->tles.size();
    }

    std::size_t get_columns_count() const override {
        return 7;
    }

    std::string get_header(std::size_t column) const override {
        return headers[column];
    }

    int get_alignment(std::size_t column) const override {
        if (column == 0) {
            return 129; // vcenter + left
        }
        return 130; // vcenter + right
    }

    std::string get_data(std::size_t row, std::size_t column) const override {
        auto &tle = _computer->tles[row];
        if (column == 0) {
            return std::format("{}", tle.t);
        }
        return std::format("{:.3f}", tle.v[column - 1]);
    }

  private:
    ballistic_computer const *_computer;
};

std::string table_data_provider_impl::headers[7]{"T", "X,м", "Y,м", "Z,м", "Vx,м/с", "Vy,м/с", "Vz,м/с"};

class tree_seance_content : public tree_data_content {
  public:
    tree_seance_content(observation_seance const &seance)
        : _seance{seance} {
    }

    std::string get_content(std::size_t column) const override {
        if (column == 0) {
            return std::to_string(_seance.id);
        } else if (column < 4) {
            return std::format("{:.3f}", _seance.o[column - 1]);
        }
        return std::string();
    }

    int get_alignment(std::size_t column) const override {
        return 130; // vcenter + right
    }

  private:
    observation_seance const &_seance;
};

class tree_measurement_content : public tree_data_content {
  public:
    tree_measurement_content(measurement_data const &measurement)
        : _measurement{measurement} {
    }

    std::string get_content(std::size_t column) const override {
        switch (column) {
        case 4:
            return std::format("{}", _measurement.t);
        case 5:
            return std::format("{:.3f}", _measurement.m);
        case 6:
            return std::format("{:.3f}", _measurement.i);
        case 7:
            return std::format("{:.3f}", _measurement.a);
        }
        return std::string();
    }

    int get_alignment(std::size_t column) const override {
        if (column == 4) {
            return 129;
        }
        return 130;
    }

  private:
    measurement_data const &_measurement;
};

class tree_root_content : public tree_data_content {
  public:
    std::string get_content(std::size_t column) const override {
        return std::string();
    }

    int get_alignment(std::size_t column) const override {
        return 128;
    }
};

class tree_data_provider_impl : public tree_data_provider {
    static std::string headers[8];

  public:
    tree_data_provider_impl(ballistic_computer const *computer)
        : _computer{computer}
        , _root{std::make_unique<tree_root_content>()} {
    }

    void update() {
        _root->clear_children();
        for (std::size_t i{}; i < _computer->seances.size(); ++i) {
            auto &seance = _computer->seances[i];
            auto child = std::make_unique<tree_seance_content>(seance);
            for (auto &measurement : seance.m) {
                child->append_child(std::make_unique<tree_measurement_content>(measurement));
            }
            _root->append_child(std::move(child));
        }
    }

    tree_data_content *get_root_content() const override {
        return _root.get();
    }

    std::string get_header(std::size_t column) const override {
        return headers[column];
    }

    std::size_t get_columns_count() const override {
        return 8;
    }

  private:
    ballistic_computer const *_computer;
    std::unique_ptr<tree_root_content> _root;
};

using namespace std::string_literals;

constexpr char degree[3]{char(0xC2), char(0xB0)};

std::string tree_data_provider_impl::headers[8]{"Обсерватория", "X,м", "Y,м", "Z,м", "T", "Блеск", "Склон.,"s + degree, "Восх.,"s + degree};

computational_model::computational_model()
    : _computer{std::make_unique<ballistic_computer>()}
    , _table_data_provider{std::make_shared<table_data_provider_impl>(_computer.get())}
    , _tree_data_provider{std::make_shared<tree_data_provider_impl>(_computer.get())} {
}

computational_model::~computational_model() = default;

std::istream &operator>>(std::istream &is, potential_harmonic &p) {
    return is >> p.cos >> p.sin;
}

void computational_model::read_gpt(std::string const &filepath) {
    auto fin = open_infile(filepath);
    egm::read_harmonics(std::istream_iterator<potential_harmonic>{fin}, {});
}

void computational_model::read_tle(std::string const &filepath) {
    _computer->tles = load_tle_observation(filepath);
}

void computational_model::read_measurements(std::string const &obs_filepath, std::string const &meas_filepath) {
    _computer->seances = load_sparkle_observation_from_json(obs_filepath, meas_filepath);
    _tree_data_provider->update();
}

void computational_model::select_tle(std::size_t number) {
    _index = number - 1;
}

void computational_model::select_interval(double interval) {
    _interval = interval;
}

std::size_t computational_model::get_tle_count() const {
    return _computer->tles.size();
}

optimization_logger const *computational_model::get_logger() const {
    return _logger.get();
}

std::shared_ptr<table_data_provider> computational_model::get_table_data_provider() const {
    return _table_data_provider;
}

std::shared_ptr<tree_data_provider> computational_model::get_tree_data_provider() const {
    return _tree_data_provider;
}

std::shared_ptr<residuals_provider> computational_model::get_residuals_provider() const {
    return _logger;
}

void computational_model::compute(const std::string &filename) {
    _logger = _computer->compute(static_cast<size_t>(_index), _interval, 20);
    if (!filename.empty()) {
        auto fout = open_outfile(filename);
        _logger->print(fout);
    }
}
