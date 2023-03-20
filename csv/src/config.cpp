#include <config.hpp>
#include <fileutility.hpp>
#include <spaceweather.hpp>
#include <formatting.hpp>
#include <curl/curl.h>
#include <filesystem>

namespace fs = std::filesystem;

void throw_if_failed(CURLcode code, char const *msg)
{
    if (code != CURLcode::CURLE_OK)
    {
        throw_runtime_error("Error occured. % Error code %.", msg, code);
    }
}

std::size_t write_file(char *buf, std::size_t size, std::size_t count, void *outstream)
{
    auto &os = *reinterpret_cast<std::ostream *>(outstream);
    if (os)
    {
        os.write(buf, size * count);
        return count;
    }
    return {};
}

class url_wrapper
{
    CURL *_url;

public:
    url_wrapper()
    {
        _url = curl_easy_init();
        if (!_url)
        {
            throw_runtime_error("Failed to initialize curl object.");
        }
        curl_easy_setopt(_url, CURLoption::CURLOPT_VERBOSE, 1L);
    }
    ~url_wrapper()
    {
        curl_easy_cleanup(_url);
    }
    void load_file(std::string const &urlstr, std::ostream &os) const
    {
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_URL, urlstr.c_str()),
                        "Failed to set option CURLOPT_URL.");
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_WRITEFUNCTION, &write_file),
                        "Failed to set option CURLOPT_WRITEFUNCTION.");
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_WRITEDATA, &os),
                        "Failed to set option CURLOPT_WRITEDATA.");
        throw_if_failed(curl_easy_perform(_url), "Failed to perform action.");
    }
};

class spaceweather_loader
{
    std::string _urlstr{"http://celestrak.org/SpaceData/SW-Last5Years.csv"};

public:
    spaceweather_loader()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    ~spaceweather_loader()
    {
        curl_global_cleanup();
    }
    void load_file(fs::path const &filepath) const
    {
        try
        {
            auto fout = open_outfile(filepath.string());
            url_wrapper url;
            url.load_file(_urlstr, fout);
        }
        catch (std::exception const &ex)
        {
            throw_runtime_error("Failed to load spaceweather data from % to file %.", _urlstr, filepath);
        }
    }
};

void read_spaceweather_from_csv(std::string const &filename);

void read_spaceweather()
{
    const fs::path filepath{"spaceweather.csv"};
    if (!fs::exists(filepath))
    {
        spaceweather_loader loader;
        loader.load_file(filepath);
    }
    read_spaceweather_from_csv(filepath.string());
}

void initialize_geopotential(std::string const &filename);

void read_geopotential(fs::path const &filepath)
{
    initialize_geopotential(filepath.string());
}

std::vector<motion_measurement> read_motion_measurements_from_txt(std::string const &);
std::vector<motion_measurement> read_motion_measurements_from_csv(std::string const &, time_type);
void write_motion_measurements_to_txt(std::string const &, std::vector<motion_measurement> const &);

auto read_measurements(fs::path const &filepath, time_type reft)
{
    auto local_path = filepath.stem();
    local_path += ".txt";
    if (fs::exists(local_path))
    {
        return read_motion_measurements_from_txt(local_path.string());
    }
    else
    {
        auto measurements = read_motion_measurements_from_csv(filepath.string(), reft);
        write_motion_measurements_to_txt(local_path.string(), measurements);
        return measurements;
    }
}

auto u8path(std::string const &filename)
{
    return fs::u8path(filename);
}

configurer::configurer(std::string const &filename)
{
    auto fin = open_infile(filename);
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
    read_geopotential(u8path(geopotential_filepath));
    read_spaceweather();
    _measurements = read_measurements(u8path(measurements_filepath), make_time(ref_time.c_str()));
    _computation_filepath = "computation.log";
}

std::string const &configurer::get_computationlog_filepath() const
{
    return _computation_filepath;
}

std::vector<motion_measurement> const &configurer::get_motion_measurements() const
{
    return _measurements;
}