#include <config.hpp>
#include <fileutils.hpp>
#include <spaceweather.hpp>
#include <geometry.hpp>
#include <iostream>

void read_spaceweather();
void read_geopotential(std::string_view);
std::vector<motion_measurement> read_motion_measurements(std::string_view, std::string_view);
std::vector<rotation_measurement> read_rotation_measurements(std::string_view, std::string_view);
std::vector<geometry> read_geometry_model_from_xml(std::string_view);

math::quaternion read_tracker_quaternion_from_file(std::string_view filepath)
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

application_configurer::application_configurer(std::string_view filepath)
{
    auto fin = open_infile(filepath);
    std::string gpt_filename;
    if (!std::getline(fin, gpt_filename))
    {
        throw std::runtime_error("Failed to read a filepath to geopotential data from config file.");
    }
    std::string asn_filename;
    if (!std::getline(fin, asn_filename))
    {
        throw std::runtime_error("Failed to read a filepath to measurements from config file.");
    }
    std::string rot_filename;
    if (!std::getline(fin, rot_filename))
    {
        throw std::runtime_error("Failed to read a filepath to rotational data.");
    }
    std::string tracker_filename;
    if (!std::getline(fin, tracker_filename))
    {
        throw std::runtime_error("Failed to read a filepath to the trakcer's properties from config file.");
    }
    std::string reftime;
    if (!std::getline(fin, reftime))
    {
        throw std::runtime_error("Failed to read reference time from config file.");
    }
    std::string geom_filename;
    if (!std::getline(fin, geom_filename))
    {
        throw std::runtime_error("Failed to read a filepath to geometry data from config file.");
    }
    read_geopotential(gpt_filename);
    read_spaceweather();
    _motion_measurements = read_motion_measurements(asn_filename, reftime);
    _rotation_measurements = read_rotation_measurements(rot_filename, reftime);
    auto tracker_q = read_tracker_quaternion_from_file(tracker_filename);
    _geometries = read_geometry_model_from_xml(geom_filename);
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