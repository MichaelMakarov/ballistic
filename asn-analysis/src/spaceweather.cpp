#include <spaceweather.hpp>
#include <fileutils.hpp>
#include <csvutility.hpp>
#include <times.hpp>
#include <vector>
#include <filesystem>
#include <format>

using namespace std::string_literals;

struct spaceweather_node
{
    double kp;
    double f10_7;
    double f81;
    time_t t;
};

static std::size_t end_column(std::string const &str, std::size_t begin)
{
    return end_column(str, begin, ',');
}

spaceweather_node read_spaceweather(std::string const &str, std::size_t row)
{
    spaceweather_node node{};
    std::size_t begin{}, end{};
    end = end_column(str, begin);
    try
    {
        time_type t = parse_from_str<parse_format::short_format>(str.substr(begin, end - begin).data());
        node.t = std::chrono::system_clock::to_time_t(t);
    }
    catch (const std::exception &ex)
    {
        throw std::runtime_error(std::format("Failed to read time from line {}. {}", row, ex.what()));
    }
    for (std::size_t i{}; i < 11; ++i)
    {
        end = end_column(str, begin = end + 1);
    }
    try
    {
        if (end - begin > 1)
            node.kp = to_double(str, begin, end) * (1e-1 / 8);
    }
    catch (std::exception const &ex)
    {
        throw std::runtime_error(std::format("Failed to read kp sum from line {}. {}", row, ex.what()));
    }
    for (std::size_t i{}; i < 14; ++i)
    {
        end = end_column(str, begin = end + 1);
    }
    try
    {
        if (end - begin > 1)
            node.f10_7 = to_double(str, begin, end);
    }
    catch (std::exception const &ex)
    {
        throw std::runtime_error(std::format("Failed to read F10.7 from line {}. {}", row, ex.what()));
    }
    for (std::size_t i{}; i < 5; ++i)
    {
        end = end_column(str, begin = end + 1);
    }
    try
    {
        if (end - begin > 1)
            node.f81 = to_double(str, begin, end);
    }
    catch (std::exception const &ex)
    {
        throw std::runtime_error(std::format("Failed to read F81 from line {}. {}", row, ex.what()));
    }
    return node;
}

constexpr auto day = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::days{1}).count();
constexpr auto hour3 = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{3}).count();

class spaceweather_handler
{
    std::vector<spaceweather_node> _nodes;
    time_t _tn, _tk;

public:
    spaceweather_handler() : _tn{}, _tk{} {}
    spaceweather_handler(std::string_view filename)
    {
        auto fin = open_infile(filename);
        std::string buf;
        if (!std::getline(fin, buf))
        {
            throw std::runtime_error("Не удалось прочитать заголовочную строку в файле "s + filename.data());
        }
        _nodes.clear();
        std::size_t row{2};
        while (std::getline(fin, buf))
        {
            _nodes.push_back(read_spaceweather(buf, row++));
        }
        _tn = _nodes.front().t;
        _tk = _nodes.back().t + day;
    }
    spaceweather_handler(spaceweather_handler &&other) noexcept : _nodes{std::move(other._nodes)}, _tn{other._tn}, _tk{other._tk}
    {
    }
    spaceweather_handler &operator=(spaceweather_handler &&other) noexcept
    {
        _nodes = std::move(other._nodes);
        _tn = other._tn;
        _tk = other._tk;
        return *this;
    }

    spaceweather get_spaceweather(time_t t) const
    {
        if (_tn > t || t > _tk)
        {
            throw std::out_of_range(std::format("Time {} of the request is out of range of space weather time interval {} - {}.",
                                                std::chrono::system_clock::from_time_t(t),
                                                std::chrono::system_clock::from_time_t(_tn),
                                                std::chrono::system_clock::from_time_t(_tk)));
        }
        std::size_t index = (t - _tn) / day;
        auto &node = _nodes[index];
        return spaceweather{.kp = node.kp, .f10_7 = node.f10_7, .f81 = node.f81};
    }
};

spaceweather_handler handler;

spaceweather get_spaceweather(time_t t)
{
    return handler.get_spaceweather(t);
}

void read_spaceweather_from_csv(std::string_view filename)
{
    handler = spaceweather_handler{filename};
}

#include <urlproc.hpp>
#include <iostream>

void read_spaceweather()
{
    constexpr std::string_view filepath{"spaceweather.csv"};
    if (!exists(filepath))
    {
        constexpr auto urlpath{"http://celestrak.org/SpaceData/SW-Last5Years.csv"};
        std::cout << "Loading spaceweather data from " << urlpath << std::endl;
        load_file_from_url(urlpath, filepath);
    }
    std::cout << "Reading spaceweather data from " << filepath << std::endl;
    read_spaceweather_from_csv(filepath);
}