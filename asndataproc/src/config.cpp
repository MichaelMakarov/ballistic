#include <config.hpp>
#include <fileutility.hpp>
#include <spaceweather.hpp>
#include <formatting.hpp>
#include <urlproc.hpp>
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
    initialize_geopotential(filepath.string());
}

std::vector<motion_measurement> read_motion_measurements_from_txt(fs::path const &);
std::vector<motion_measurement> read_motion_measurements_from_csv(fs::path const &, time_point_t);
void write_motion_measurements_to_txt(fs::path const &, std::vector<motion_measurement> const &);

auto read_measurements(fs::path const &filepath, std::string const &reft_str)
{
    auto local_path = filepath.stem();
    local_path += ".txt";
    if (fs::exists(local_path))
    {
        std::cout << "Reading measurements from " << local_path << std::endl;
        return read_motion_measurements_from_txt(local_path.string());
    }
    else
    {
        std::cout << "reading measurements from " << filepath << " using reference time " << reft_str << std::endl;
        time_point_t t = parse_from_str<parse_format::long_format>(reft_str);
        auto measurements = read_motion_measurements_from_csv(filepath.string(), t);
        write_motion_measurements_to_txt(local_path.string(), measurements);
        return measurements;
    }
}

configurer::configurer(fs::path const &filepath)
{
    auto fin = open_infile(filepath);
    std::string geopotential_filepath, measurements_filepath, ref_time;
    if (!std::getline(fin, geopotential_filepath))
    {
        throw_runtime_error("Failed to read a filepath to geopotential data from config file.");
    }
    if (!std::getline(fin, measurements_filepath))
    {
        throw_runtime_error("Failed to read a filepath to measurements from config file.");
    }
    if (!std::getline(fin, ref_time))
    {
        throw_runtime_error("Failed to read reference time from config file.");
    }
    read_geopotential(path_from_utf8(geopotential_filepath));
    read_spaceweather();
    _measurements = read_measurements(path_from_utf8(measurements_filepath), ref_time);
    _computation_filepath = "computation.log";
    std::cout << "Computational info will be written to " << _computation_filepath << std::endl;
}
