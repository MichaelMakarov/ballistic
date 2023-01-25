#include <mainmodel.hpp>
#include <computation.hpp>
#include <formatoutput.hpp>
#include <formatting.hpp>
#include <motion.hpp>
#include <rotation.hpp>
#include <settings.hpp>
#include <ball.hpp>
#include <fstream>

constexpr double sec_per_day{86400};

std::ofstream open_outfile(const std::string_view);

/**
 * @brief Настройки вычисления.
 */
struct settings_context
{
    project_settings prj{};
    computational_output out;
    double inter{};
    size_t tle{};
    size_t stage{};
};

bool operator==(const object_info &left, const object_info &right)
{
    return left.mass == right.mass;
}

bool operator==(const project_settings &left, const project_settings &right)
{
    bool flag{true};
    flag &= left.gptpath == right.gptpath;
    flag &= left.mespath == right.mespath;
    flag &= left.obspath == right.obspath;
    flag &= left.tlepath == right.tlepath;
    flag &= left.object == right.object;
    return flag;
}

bool operator==(const settings_context &left, const settings_context &right)
{
    bool flag{true};
    flag &= left.inter == right.inter;
    flag &= left.tle == right.tle;
    flag &= left.prj == right.prj;
    return flag;
}

class context_binding
{
    settings_context *_ctx;
    std::string_view _filename;

public:
    context_binding(settings_context &ctx, std::string_view filename) : _ctx{&ctx}, _filename{filename} {}
    context_binding(const context_binding &) noexcept = default;
    context_binding &operator=(const context_binding &) noexcept = default;
    ~context_binding() noexcept(false)
    {
        using namespace std::string_literals;
        if (_filename.empty())
            return;
        try
        {
            auto fout = open_outfile(_filename);
            print_output(fout, _ctx->out, true);
        }
        catch (const std::exception &ex)
        {
            throw std::runtime_error("При сохранении результатов в файл возникла ошибка. "s + ex.what());
        }
    }
    computational_output &output() const { return _ctx->out; }
    size_t &stage() const { return _ctx->stage; }
};

/**
 * @brief Объект для хранения контекста выполнения
 */
class computational_context
{
    /**
     * @brief Список всех контекстов вычисления.
     */
    std::list<settings_context> contexts;

public:
    context_binding bind(double inter, size_t tle, std::string_view filename)
    {
        settings_context ctx{
            .prj = settings,
            .inter = inter,
            .tle = tle};
        auto iter = std::ranges::find(contexts, ctx);
        if (iter != std::ranges::end(contexts))
        {
            return context_binding(*iter, filename);
        }
        else
        {
            contexts.push_back(ctx);
            return context_binding(contexts.back(), filename);
        }
    }
};

bool cmp_to_be_earlier(const observation_seance &elem, time_h ref)
{
    return ref > elem.m.front().t;
}

bool cmp_to_be_later(time_h ref, const observation_seance &elem)
{
    return ref < elem.m.back().t;
}

class ballistic_computer
{
public:
    /**
     * @brief Массив данных считанных ТЛЕ
     *
     */
    std::vector<orbit_data> tles;
    /**
     * @brief Массив считанных сеансов измерений
     *
     */
    std::vector<observation_seance> seances;

public:
    /**
     * @brief Решение задачи оптимизации на мерном интервале
     *
     * @param tle_index индекс опорного ТЛЕ
     * @param interval длина мерного интервала в сек
     */
    void compute(computational_output &output, size_t tle_index, double interval)
    {
        verify();
        // начальные условия из ТЛЕ
        output.refer = std::make_shared<orbit_data>();
        *output.refer = tles.at(tle_index);
        // начальный момент времени
        auto tn = output.refer->t;
        // конечный момент времени
        auto tk = tn + interval;
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
            throw_runtime_error("Недостаточное кол-во измерений % на интервале % - %.", number, tn, tk);
        }
        // мерный интервал
        output.inter = std::make_shared<measuring_interval>(inter);
        auto func = [&output, tn](std::size_t i)
        {
            if (i != 0)
                return;
            if (i == 0)
            {
                // решение по базовой модели
                output.basic = std::make_shared<basic_info>();
                std::copy(output.refer->v, output.refer->v + 6, output.basic->v.data());
                estimate_model(*output.inter, tn, *output.basic);
            }
            else if (i == 1)
            {
                round_plane plane{
                    .mass = settings.object.mass,
                    .square = settings.object.square,
                    .refl = settings.object.refl};
                output.extbasic = std::make_shared<extbasic_info>();
                std::copy(output.refer->v, output.refer->v + 6, output.extbasic->v.data());
                estimate_model(*output.inter, tn, plane, *output.extbasic);
            }
            else
            {
                // оценка параметров вращения
                rotation_info info{};
                estimate_rotation(*output.inter, output.refer->v, tn, info);
                output.rotation = std::make_shared<rotation_info>();
                *output.rotation = info;
                // параметры диска
                round_plane plane{
                    .mass = settings.object.mass,
                    .square = settings.object.square,
                    .refl = settings.object.refl,
                    .normal = output.rotation->n};
                output.extended = std::make_shared<extended_info>();
                std::copy(output.refer->v, output.refer->v + 6, output.extended->v.data());
                // решение по модели с учетом вращения
                estimate_model(*output.inter, tn, plane, output.rotation->r, *output.extended);
            }
        };
        par::parallel_for(std::size_t{}, std::size_t{3}, func);
    }

private:
    void verify() const
    {
        if (egm::harmonics.empty())
        {
            throw_runtime_error("Гармоники геопотенциала Земли не загружены.");
        }
        if (tles.empty())
        {
            throw_runtime_error("Данные TLE отсутствуют.");
        }
        if (seances.empty())
        {
            throw_runtime_error("Данные измерений отсутствуют.");
        }
    }
};

computational_model::computational_model(QObject *parent) : QObject(parent)
{
    _computer = new ballistic_computer;
    _context = new computational_context;
}

computational_model::~computational_model()
{
    delete _computer;
    delete _context;
}

std::istream &operator>>(std::istream &is, potential_harmonic &p)
{
    return is >> p.cos >> p.sin;
}

std::ifstream open_infile(const std::string_view filename);

void computational_model::read_gpt() const
{
    auto fin = open_infile(settings.gptpath);
    egm::read_harmonics(std::istream_iterator<potential_harmonic>{fin}, {});
}

void computational_model::read_tle() const
{
    _computer->tles = load_orbit_data(settings.tlepath);
    emit tle_data_loaded();
}

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

void computational_model::compute(const std::string &filename) const
{
    using namespace std::string_literals;
    std::exception_ptr exptr;
    try
    {
        auto binding = _context->bind(_interval, _index, filename);
        try
        {
            _computer->compute(binding.output(), _index, _interval * sec_per_day);
        }
        catch (const std::exception &)
        {
            exptr = std::current_exception();
        }
        emit computation_performed(&binding.output());
    }
    catch (const std::exception &)
    {
        // because destructor may throw exception
        exptr = std::current_exception();
    }
    if (exptr)
        std::rethrow_exception(exptr);
}
