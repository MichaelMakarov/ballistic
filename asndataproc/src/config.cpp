#include <config.hpp>
#include <fileutility.hpp>
#include <spaceweather.hpp>
#include <urlproc.hpp>
#include <geometry.hpp>
#include <iostream>

void read_spaceweather_from_csv(fs::path const &filepath);

void read_spaceweather()
{
    const fs::path filepath{"spaceweather.csv"};
    if (!fs::exists(filepath))
    {
        fs::path const urlpath{"http://celestrak.org/SpaceData/SW-Last5Years.csv"};
        std::cout << "Loading spaceweather data from " << urlpath << std::endl;
        load_file_from_url(urlpath, filepath);
    }
    std::cout << "Reading spaceweather data from " << filepath << std::endl;
    read_spaceweather_from_csv(filepath);
}

void initialize_geopotential(fs::path const &filepath);

void read_geopotential(fs::path const &filepath)
{
    std::cout << "Reading geopotential data from " << filepath << std::endl;
    initialize_geopotential(filepath);
}

std::vector<motion_measurement> read_motion_measurements_from_txt(fs::path const &);
std::vector<motion_measurement> read_motion_measurements_from_csv(fs::path const &, time_point_t);
void write_motion_measurements_to_txt(fs::path const &, std::vector<motion_measurement> const &);

auto read_motion_measurements(fs::path const &filepath, std::string const &reft_str)
{
    auto local_path = filepath.stem();
    local_path += ".txt";
    if (fs::exists(local_path))
    {
        std::cout << "Reading motion measurements from " << local_path << std::endl;
        return read_motion_measurements_from_txt(local_path.string());
    }
    else
    {
        std::cout << "Reading motion measurements from " << filepath << " using reference time " << reft_str << std::endl;
        time_point_t t = parse_from_str<parse_format::long_format>(reft_str);
        auto measurements = read_motion_measurements_from_csv(filepath.string(), t);
        write_motion_measurements_to_txt(local_path.string(), measurements);
        return measurements;
    }
}

std::vector<rotation_measurement> read_rotation_measurements_from_csv(fs::path const &filepath, time_point_t reft);
std::vector<rotation_measurement> read_rotation_measurements_from_txt(fs::path const &filepath);
void write_rotation_measurements_to_txt(fs::path const &filepath, std::vector<rotation_measurement> const &measurements);

auto read_rotation_measurements(fs::path const &filepath, std::string const &reft_str)
{
    auto local_path = filepath.stem();
    local_path += ".txt";
    if (fs::exists(local_path))
    {
        std::cout << "Reading rotation measurements from " << local_path << std::endl;
        return read_rotation_measurements_from_txt(local_path.string());
    }
    else
    {
        std::cout << "Reading rotation measurements from " << filepath << " using reference time " << reft_str << std::endl;
        time_point_t t = parse_from_str<parse_format::long_format>(reft_str);
        auto measurements = read_rotation_measurements_from_csv(filepath.string(), t);
        write_rotation_measurements_to_txt(local_path.string(), measurements);
        return measurements;
    }
}

math::quaternion read_tracker_quaternion_from_file(fs::path const &filepath)
{
    std::cout << "Reading quaternion from " << filepath << std::endl;
    auto fin = open_infile(filepath);
    double angles[3];
    fin >> angles[0] >> angles[1] >> angles[2];
    for (std::size_t i{}; i < 3; ++i)
        angles[i] = math::deg_to_rad(angles[i]);
    math::quaternion q;
    math::quaternion::angles_to_quat(angles[0], angles[1], angles[2], q, math::RotationOrder::yxz);
    return q;
}

std::vector<geometry> read_geometry_model_from_xml(fs::path const &filepath);

application_configurer::application_configurer(fs::path const &filepath)
{
    auto fin = open_infile(filepath);
    std::string geopotential_filepath;
    if (!std::getline(fin, geopotential_filepath))
    {
        throw std::runtime_error("Failed to read a filepath to geopotential data from config file.");
    }
    std::string measurements_filepath;
    if (!std::getline(fin, measurements_filepath))
    {
        throw std::runtime_error("Failed to read a filepath to measurements from config file.");
    }
    std::string rotation_filepath;
    if (!std::getline(fin, rotation_filepath))
    {
        throw std::runtime_error("Failed to read a filepath to rotational data.");
    }
    std::string tracker_filepath;
    if (!std::getline(fin, tracker_filepath))
    {
        throw std::runtime_error("Failed to read a filepath to the trakcer's properties from config file.");
    }
    std::string ref_time;
    if (!std::getline(fin, ref_time))
    {
        throw std::runtime_error("Failed to read reference time from config file.");
    }
    std::string geometry_filepath;
    if (!std::getline(fin, geometry_filepath))
    {
        throw std::runtime_error("Failed to read a filepath to geometry data from config file.");
    }
    read_geopotential(path_from_utf8(geopotential_filepath));
    read_spaceweather();
    _motion_measurements = read_motion_measurements(path_from_utf8(measurements_filepath), ref_time);
    _rotation_measurements = read_rotation_measurements(path_from_utf8(rotation_filepath), ref_time);
    auto tracker_q = read_tracker_quaternion_from_file(path_from_utf8(tracker_filepath));
    _geometries = read_geometry_model_from_xml(path_from_utf8(geometry_filepath));
    _computation_filepath = "computation.log";
    std::cout << "Computational info will be written to " << _computation_filepath << std::endl;
    for (auto &m : _rotation_measurements)
    {
        m.q = inverse(m.q);
        m.q = mul(tracker_q, m.q);
    }
    set_iterators();
}

void application_configurer::set_iterators()
{
    auto tn = std::max(_motion_measurements.front().t, _rotation_measurements.front().t);
    auto tk = std::min(_motion_measurements.back().t, _rotation_measurements.back().t);
    if (tn >= tk)
    {
        throw std::runtime_error("Motion and rotation measurements' time intervals are incompatible.");
    }
    auto lower_compare = [](auto const &m, auto t)
    {
        return m.t < t;
    };
    auto upper_compare = [](auto t, auto const &m)
    {
        return t < m.t;
    };
    _mbegin = std::lower_bound(std::begin(_motion_measurements), std::end(_motion_measurements), tn, lower_compare);
    _mend = std::upper_bound(std::begin(_motion_measurements), std::end(_motion_measurements), tk, upper_compare);
    _rbegin = std::lower_bound(std::begin(_rotation_measurements), std::end(_rotation_measurements), tn, lower_compare);
    _rend = std::upper_bound(std::begin(_rotation_measurements), std::end(_rotation_measurements), tk, upper_compare);
}