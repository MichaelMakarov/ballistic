#pragma once

#include <assertion.h>

#include <qstring.h>

class file_logger : public basic_logger {
public:
    file_logger();
    file_logger(const QString& filename);
    file_logger(file_logger&& other) noexcept;

    file_logger& operator=(file_logger&& other) noexcept;

    std::streambuf* rdbuf();
};