#include <logger.hpp>
#include <residuals_iterator.hpp>

namespace
{
    std::size_t constexpr _res_size{2};

    template <std::size_t _index>
    class residuals_vector_iterator
    {
        math::vector const *_v;
        std::size_t _offset;

    public:
        residuals_vector_iterator(math::vector const &v, std::size_t offset) : _v{&v}, _offset{offset}
        {
        }
        residuals_vector_iterator(residuals_vector_iterator const &) = default;
        residuals_vector_iterator &operator=(residuals_vector_iterator const &) = default;
        residuals_vector_iterator &operator++()
        {
            _offset += _res_size;
            return *this;
        }
        double operator*() const
        {
            return (*_v)[_offset + _index];
        }
        bool operator!=(residuals_vector_iterator const &other) const
        {
            return _v != other._v || _offset != other._offset;
        }
    };

    std::ostream &operator<<(std::ostream &os, math::vector const &v)
    {
        for (std::size_t i{}; i < v.size(); ++i)
        {
            os << v[i] << ' ';
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, math::iteration const &iter)
    {
        os << "Итерация №" << iter.n << '\n';
        os << "Ф-ция невязок " << math::rad_to_deg(iter.r) << '\n';
        os << "Вектор параметров " << iter.v << '\n';
        os << "Вектор поправок " << iter.dv << '\n';
        return os;
    }

    void print_stat(std::ostream &os, char const *name, double mean, double std)
    {
        os << name << " \t среднее значение " << math::rad_to_deg(mean) << "град \t СКО " << math::rad_to_deg(std) << "град\n";
    }

    void print_stat_info(std::ostream &os, math::iteration const &iter)
    {
        double mean, std;
        math::mean_std(mean, std, residuals_vector_iterator<0>(iter.rv, 0), residuals_vector_iterator<0>(iter.rv, iter.rv.size()));
        print_stat(os, "Склонение:\t", mean, std);
        math::mean_std(mean, std, residuals_vector_iterator<1>(iter.rv, 0), residuals_vector_iterator<1>(iter.rv, iter.rv.size()));
        print_stat(os, "Восхождение:\t", mean, std);
    }

    template <typename input_iterator, typename output_iterator>
    void copy(input_iterator in_begin, input_iterator in_end, output_iterator out_begin) {
        while (in_begin != in_end) {
            *out_begin = *in_begin;
            ++out_begin;
            ++in_begin;
        }
    }

    std::vector<residual_point> make_residuals_array(math::vector const &rv) 
    {
        std::vector<residual_point> residuals(rv.size() / _res_size);
        copy(residuals_vector_iterator<0>{rv, 0}, residuals_vector_iterator<0>{rv, rv.size()}, residuals_iterator<&residual_point::a>{residuals.data()});
        copy(residuals_vector_iterator<1>{rv, 0}, residuals_vector_iterator<1>{rv, rv.size()}, residuals_iterator<&residual_point::i>{residuals.data()});
        return residuals;
    }
}

optimization_logger::optimization_logger(std::size_t max_iter_count, measuring_interval const &inter) : _interval{inter}
{
    _iterations.reserve(max_iter_count);
}

void optimization_logger::save(math::iteration &&iter)
{
    _iterations.push_back(std::move(iter));
}

void optimization_logger::print(std::ostream &os) const
{
    os << std::fixed << std::chrono::system_clock::now() << " Начало записи протокола вычислений.\n\n";

    os << "Мерный интервал: " << _interval.tn() << " - " << _interval.tk() << " " << _interval.points_count() << " измерений.\n";
    os << "Статистические параметры невязок.\n";
    if (!_iterations.empty())
    {
        os << "На первой итерации\n";
        print_stat_info(os, _iterations.front());
        os << "На последней итерации (" << _iterations.back().n << ")\n";
        print_stat_info(os, _iterations.back());
    }
    for (auto &iter : _iterations)
    {
        os << iter << '\n';
    }
    os << "\n\nОкончание записи протокола вычислений. " << std::chrono::system_clock::now();
}

std::vector<residual_point> optimization_logger::get_first_iteration_residuals() const
{
    return make_residuals_array(_iterations.front().rv);
}

std::vector<residual_point> optimization_logger::get_last_iteration_residuals() const
{
    return make_residuals_array(_iterations.back().rv);
}
