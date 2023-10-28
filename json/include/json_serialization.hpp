#pragma once

#include <fileutils.hpp>

#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include <type_traits>

template <typename T>
void serialize_to_json(std::string_view filename, T const &obj) {
    auto buf = yas::save<yas::mem | yas::json>(obj);
    auto os = open_outfile(filename, std::ios_base::out);
    os.write(buf.data.get(), buf.size);
}

template <typename T>
std::enable_if_t<std::is_default_constructible_v<T>, T> deserialize_from_json(std::string_view filename) {
    auto is = open_infile(filename, std::ios_base::in | std::ios_base::ate);
    std::size_t size = is.tellg();
    is.seekg(0, std::ios_base::beg);
    yas::shared_buffer buf{size};
    is.read(buf.data.get(), size);
    T obj;
    yas::load<yas::mem | yas::json>(buf, obj);
    return obj;
}