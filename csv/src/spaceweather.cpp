#include <spaceweather.hpp>
#include <fileutility.hpp>
#include <formatting.hpp>
#include <csvutility.hpp>
#include <vector>

struct spaceweather_node
{
    double kp[8];
    double f10_7;
    double f81;
    time_t t;
};

static std::size_t field_end(std::string const &str, std::size_t begin)
{
    return field_end(str, begin, ',');
}

spaceweather_node read_spaceweather(std::string const &str, std::size_t row)
{
    spaceweather_node node{};
    std::size_t begin{}, end{};
    end = field_end(str, begin);
    try
    {
        auto substr = str.substr(begin, end - begin);
        node.t = make_time(substr.c_str(), "y-m-d");
    }
    catch (const std::exception &ex)
    {
        throw_runtime_error("Failed to read time from line %. %", row, ex.what());
    }
    for (std::size_t i{}; i < 2; ++i)
    {
        end = field_end(str, begin = end + 1);
    }
    for (std::size_t i{}; i < 8; ++i)
    {
        end = field_end(str, begin = end + 1);
        try
        {
            if (end - begin > 1)
                node.kp[i] = to_double(str, begin, end);
        }
        catch (std::exception const &ex)
        {
            throw_runtime_error("Failed to read kp[%] from line %. %", i + 1, row, ex.what());
        }
    }
    for (std::size_t i{}; i < 15; ++i)
    {
        end = field_end(str, begin = end + 1);
    }
    try
    {
        if (end - begin > 1)
            node.f10_7 = to_double(str, begin, end);
    }
    catch (std::exception const &ex)
    {
        throw_runtime_error("Failed to read F10.7 from line %. %", row, ex.what());
    }
    for (std::size_t i{}; i < 5; ++i)
    {
        end = field_end(str, begin = end + 1);
    }
    try
    {
        if (end - begin > 1)
            node.f81 = to_double(str, begin, end);
    }
    catch (std::exception const &ex)
    {
        throw_runtime_error("Failed to read F81 from line %. %", row, ex.what());
    }
    return node;
}

constexpr auto day = make_days(1);
constexpr auto hour3 = make_hour(3);

class spaceweather_handler
{
    std::vector<spaceweather_node> _nodes;
    time_type _tn, _tk;

public:
    spaceweather_handler() : _tn{}, _tk{} {}
    spaceweather_handler(std::string const &filename)
    {
        auto fin = open_infile(filename);
        std::string buf;
        if (!std::getline(fin, buf))
        {
            throw_runtime_error("Failed to read header from file " + filename);
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

    spaceweather get_spaceweather(time_type t) const
    {
        if (_tn > t || t > _tk)
        {
            throw_out_of_range("Time % of the request is out of range of space weather time interval % - %.",
                               calendar{t}, calendar{_tn}, calendar{_tk});
        }
        std::size_t index = (t - _tn) / day;
        auto &node = _nodes[index];
        index = (t - node.t) / hour3;
        return spaceweather{.kp = node.kp[index], .f10_7 = node.f10_7, .f81 = node.f81};
    }
};

spaceweather_handler handler;

spaceweather get_spaceweather(time_type t)
{
    return handler.get_spaceweather(t);
}

void read_spaceweather_from_csv(std::string const &filename)
{
    handler = spaceweather_handler{filename};
}