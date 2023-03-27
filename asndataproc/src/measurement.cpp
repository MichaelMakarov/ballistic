#include <formatting.hpp>
#include <fileutility.hpp>
#include <csvutility.hpp>
#include <measurement.hpp>
#include <vector>
#include <filesystem>

constexpr char const *motion_headers[6]{"x", "y", "z", "vx", "vy", "vz"};

bool compare(motion_measurement const &left, motion_measurement const &right)
{
    if (left.t == right.t)
        return true;
    for (std::size_t i{}; i < 6; ++i)
    {
        if (left.v[i] == right.v[i])
            return true;
    }
    return false;
}

static std::size_t field_end(std::string const &str, std::size_t begin)
{
    return field_end(str, begin, ';');
}

std::vector<motion_measurement> read_motion_measurements_from_csv(fs::path const &filename, time_point_t tn)
{
    std::ifstream fin = open_infile(filename);
    std::string buf;
    if (!std::getline(fin, buf))
    {
        throw_runtime_error("Failed to read a header string from file %", filename.string());
    }
    std::vector<motion_measurement> measurements;
    motion_measurement old;
    for (std::size_t number{2}; std::getline(fin, buf); ++number)
    {
        motion_measurement m;
        m.t = tn;
        std::size_t count{}, begin{}, end{};
        // проматываем первые 18 полей
        while (count < 16)
        {
            end = field_end(buf, begin = end + 1);
            ++count;
        }
        // поле с секундами
        end = field_end(buf, begin = end + 1);
        try
        {
            auto t = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::seconds(t);
        }
        catch (std::invalid_argument const &)
        {
            throw_invalid_argument("Invalid format of string with seconds in row %.", number);
        }
        // пропускаем поле
        end = field_end(buf, begin = end + 1);
        // поле с фиксированным временем, где нужно взять сутки
        end = field_end(buf, begin = end + 1);
        try
        {
            auto day = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::days(day);
        }
        catch (std::invalid_argument const &)
        {
            throw_runtime_error("Invalid format of the string with time in row %.", number);
        }
        // поле с доп. секундой
        end = field_end(buf, begin = end + 1);
        try
        {
            auto sec = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::seconds(sec);
        }
        catch (std::invalid_argument const &)
        {
            throw_runtime_error("Invalid format of the string width additional second in row %.", number);
        }
        // поля с координатами и скоростями
        for (std::size_t i{}; i < 2; ++i)
        {
            // пустое поле
            end = field_end(buf, begin = end + 1);
            for (std::size_t j{}; j < 3; ++j)
            {
                auto index = i * 3 + j;
                end = field_end(buf, begin = end + 1);
                try
                {
                    m.v[index] = std::stod(buf.substr(begin, end - begin));
                }
                catch (std::exception const &ex)
                {
                    throw_runtime_error("Invalid format of coordinate % value in line %. %", motion_headers[index], number, ex.what());
                }
            }
        }
        if (!compare(old, m))
        {
            measurements.push_back(m);
            old = m;
        }
    }
    return measurements;
}

std::ostream &operator<<(std::ostream &os, motion_measurement const &m)
{
    write_to_stream(os, m.t);
    for (std::size_t i{}; i < 6; ++i)
    {
        os << ' ' << m.v[i];
    }
    return os;
}

bool read_measurement(std::istream &is, motion_measurement &m)
{
    std::string str;
    is >> str;
    if (!str.empty())
    {
        m.t = parse_from_str<parse_format::long_format>(str);
        for (std::size_t i{}; i < 6; ++i)
        {
            is >> m.v[i];
        }
        return true;
    }
    return false;
}

std::vector<motion_measurement> read_motion_measurements_from_txt(fs::path const &filename)
{
    std::vector<motion_measurement> measurements;
    auto fin = open_infile(filename);
    while (!fin.eof())
    {
        motion_measurement m;
        if (read_measurement(fin, m))
            measurements.push_back(m);
    }
    return measurements;
}

void write_motion_measurements_to_txt(fs::path const &filename, std::vector<motion_measurement> const &measurements)
{
    auto fout = open_outfile(filename);
    fout << std::fixed;
    std::copy(std::begin(measurements), std::end(measurements), std::ostream_iterator<motion_measurement>{fout, "\n"});
}