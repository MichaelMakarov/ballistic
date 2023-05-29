#include <urlproc.hpp>
#include <fileutils.hpp>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>

void throw_if_failed(CURLcode code, char const *msg)
{
    if (code != CURLcode::CURLE_OK)
    {
        std::stringstream sstr;
        sstr << msg << " Error code " << code << '.';
        throw std::runtime_error(sstr.str());
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
            throw std::runtime_error("Failed to initialize curl object.");
        }
        curl_easy_setopt(_url, CURLoption::CURLOPT_VERBOSE, 1L);
    }
    ~url_wrapper()
    {
        curl_easy_cleanup(_url);
    }
    void load_file(std::string_view urlpath, std::ostream &os) const
    {
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_URL, urlpath.data()),
                        "Failed to set option CURLOPT_URL.");
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_WRITEFUNCTION, &write_file),
                        "Failed to set option CURLOPT_WRITEFUNCTION.");
        throw_if_failed(curl_easy_setopt(_url, CURLoption::CURLOPT_WRITEDATA, &os),
                        "Failed to set option CURLOPT_WRITEDATA.");
        throw_if_failed(curl_easy_perform(_url), "Failed to perform action.");
    }
};

struct fileloader
{
    fileloader()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    ~fileloader()
    {
        curl_global_cleanup();
    }
    void load_file(std::string_view urlpath, std::string_view filepath) const
    {
        try
        {
            auto fout = open_outfile(filepath);
            url_wrapper{}.load_file(urlpath, fout);
        }
        catch (std::exception const &ex)
        {
            using namespace std::string_literals;
            throw std::runtime_error("Не удалось скачать файл по url = "s + urlpath.data() + ". "s + ex.what());
        }
    }
};

void load_file_from_url(std::string_view urlpath, std::string_view filepath)
{
    fileloader{}.load_file(urlpath, filepath);
}