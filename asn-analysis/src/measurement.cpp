#include <fileutils.hpp>
#include <csvutility.hpp>
#include <measurement.hpp>
#include <vector>
#include <filesystem>
#include <format>

using namespace std::string_literals;

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

static std::size_t end_column(std::string const &str, std::size_t begin)
{
    return end_column(str, begin, ';');
}

std::vector<motion_measurement> read_motion_measurements_from_csv(std::string_view filename, time_type tn)
{
    std::ifstream fin = open_infile(filename);
    std::string buf;
    if (!std::getline(fin, buf))
    {
        throw std::runtime_error("Failed to read header line from "s + filename.data());
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
            end = end_column(buf, begin = end + 1);
            ++count;
        }
        // поле с секундами
        end = end_column(buf, begin = end + 1);
        try
        {
            auto t = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::seconds(t);
        }
        catch (std::invalid_argument const &)
        {
            throw std::invalid_argument(std::format("Invalid format of seconds in row {}.", number));
        }
        // пропускаем поле
        end = end_column(buf, begin = end + 1);
        // поле с фиксированным временем, где нужно взять сутки
        end = end_column(buf, begin = end + 1);
        try
        {
            auto day = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::days(day);
        }
        catch (std::invalid_argument const &)
        {
            throw std::runtime_error(std::format("Invlaid time's format in row {}.", number));
        }
        // поле с доп. секундой
        end = end_column(buf, begin = end + 1);
        try
        {
            auto sec = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::seconds(sec);
        }
        catch (std::invalid_argument const &)
        {
            throw std::runtime_error(std::format("Invalid second's format in row {}.", number));
        }
        // поля с координатами и скоростями
        for (std::size_t i{}; i < 2; ++i)
        {
            // пустое поле
            end = end_column(buf, begin = end + 1);
            for (std::size_t j{}; j < 3; ++j)
            {
                auto index = i * 3 + j;
                end = end_column(buf, begin = end + 1);
                try
                {
                    m.v[index] = std::stod(buf.substr(begin, end - begin));
                }
                catch (std::exception const &ex)
                {
                    throw std::runtime_error(std::format("Invalid coordinate {} format in row {}. {}", motion_headers[index], number, ex.what()));
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
        m.t = parse_from_str<parse_format::long_format>(str.data());
        for (std::size_t i{}; i < 6; ++i)
        {
            is >> m.v[i];
        }
        return true;
    }
    return false;
}

std::vector<motion_measurement> read_motion_measurements_from_txt(std::string_view filename)
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

void write_motion_measurements_to_txt(std::string_view filename, std::vector<motion_measurement> const &measurements)
{
    auto fout = open_outfile(filename);
    fout << std::fixed;
    std::copy(std::begin(measurements), std::end(measurements), std::ostream_iterator<motion_measurement>{fout, "\n"});
}

std::vector<rotation_measurement> read_rotation_measurements_from_csv(std::string_view filepath, time_type reft)
{
    auto fin = open_infile(filepath);
    std::vector<rotation_measurement> measurements;
    std::string buf;
    if (!std::getline(fin, buf))
    {
        throw std::runtime_error("Failed to read header line from "s + filepath.data());
    }
    for (std::size_t number{2}; std::getline(fin, buf); ++number)
    {
        rotation_measurement m{.t = reft};
        std::size_t begin{}, end{};
        end = end_column(buf, begin);
        end = end_column(buf, begin = end + 1);
        try
        {
            auto sec = std::stoll(buf.substr(begin, end - begin));
            m.t += std::chrono::seconds{sec};
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument(std::format("Failed to read second in row {}.", number));
        }
        begin = end + 1;
        double q[4];
        for (std::size_t i{}; i < 4; ++i)
        {
            end = end_column(buf, begin);
            try
            {
                q[i] = std::stod(buf.substr(begin, end - begin));
            }
            catch (std::invalid_argument const &)
            {
                throw std::invalid_argument(std::format("Failed to read q[{}] in row {}.", i, number));
            }
            begin = end + 1;
        }
        m.q = math::quaternion{q[1], q[2], q[3], q[0]};
        measurements.push_back(m);
    }
    return measurements;
}

std::ostream &operator<<(std::ostream &os, rotation_measurement const &m)
{
    write_to_stream(os, m.t);
    os << ' ' << m.q.s() << ' ' << m.q.x() << ' ' << m.q.y() << ' ' << m.q.z();
    return os;
}

bool read_measurement(std::istream &is, rotation_measurement &m)
{
    std::string str;
    is >> str;
    if (!str.empty())
    {
        m.t = parse_from_str<parse_format::long_format>(str.data());
        double q[4];
        for (std::size_t i{}; i < 4; ++i)
        {
            is >> q[i];
        }
        m.q = math::quaternion(q[0], q[1], q[2], q[3]);
        return true;
    }
    return false;
}

std::vector<rotation_measurement> read_rotation_measurements_from_txt(std::string_view filepath)
{
    std::vector<rotation_measurement> measurements;
    auto fin = open_infile(filepath);
    while (!fin.eof())
    {
        rotation_measurement m;
        if (!read_measurement(fin, m))
        {
            break;
        }
        measurements.push_back(m);
    }
    return measurements;
}

void write_rotation_measurements_to_txt(std::string_view filepath, std::vector<rotation_measurement> const &measurements)
{
    auto fout = open_outfile(filepath);
    fout << std::fixed;
    std::copy(std::begin(measurements), std::end(measurements), std::ostream_iterator<rotation_measurement>{fout, "\n"});
}

std::string make_local_txtname(std::string_view csvname)
{
    auto begin = std::max(std::strrchr(csvname.data(), '/'), std::strrchr(csvname.data(), '\\'));
    // если строка не содержит слэшей
    if (!begin)
        begin = csvname.data();
    else
        ++begin;
    auto end = std::strrchr(begin, '.');
    return std::string(begin, end - begin) + ".txt";
}

#include <iostream>

std::vector<motion_measurement> read_motion_measurements(std::string_view filename, std::string_view reftstr)
{
    auto txtname = make_local_txtname(filename);
    if (exists(txtname))
    {
        std::cout << "Reading motion measurements from " << txtname << std::endl;
        return read_motion_measurements_from_txt(txtname);
    }
    else
    {
        std::cout << "Reading motion measurements from " << filename << " using reference time " << reftstr << std::endl;
        time_type t = parse_from_str<parse_format::long_format>(reftstr.data());
        auto measurements = read_motion_measurements_from_csv(filename, t);
        write_motion_measurements_to_txt(txtname, measurements);
        return measurements;
    }
}

std::vector<rotation_measurement> read_rotation_measurements(std::string_view filename, std::string_view reftstr)
{
    auto txtname = make_local_txtname(filename);
    if (exists(txtname))
    {
        std::cout << "Reading rotation measurements from " << txtname << std::endl;
        return read_rotation_measurements_from_txt(txtname);
    }
    else
    {
        std::cout << "Reading rotation measurements from " << filename << " using reference time " << reftstr << std::endl;
        time_type t = parse_from_str<parse_format::long_format>(reftstr.data());
        auto measurements = read_rotation_measurements_from_csv(filename, t);
        write_rotation_measurements_to_txt(txtname, measurements);
        return measurements;
    }
}