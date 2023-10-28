#include <settings.hpp>

#include <json_serialization.hpp>


template <typename archive>
void serialize(archive &ar, project_settings &s) {
    ar &YAS_OBJECT_NVP("configuration", ("gpt", s.gptpath), ("tle", s.tlepath), ("observatories", s.obspath), ("measurements", s.mespath));
}

project_settings load_project_settings_from_json(std::string_view filename) {
    return deserialize_from_json<project_settings>(filename);
}

void save_project_settings_to_json(std::string_view filename, project_settings const &settings) {
    serialize_to_json<project_settings>(filename, settings);
}
