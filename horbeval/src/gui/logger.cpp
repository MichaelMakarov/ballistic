#include <logger.h>
#include <fstream>

std::unique_ptr<std::ofstream> open_file(const QString& filename)
{
    auto filepath = filename.toStdString();
    auto ptr = std::make_unique<std::ofstream>(filepath);
    ASSERT(ptr->is_open(), "Не удалось открыть для записи файл " + filepath);
    return ptr;
}

file_logger::file_logger() : basic_logger(std::unique_ptr<std::ostream>{})
{}

file_logger::file_logger(const QString& filename) : basic_logger(open_file(filename)) 
{}

file_logger::file_logger(file_logger&& other) noexcept : basic_logger(std::forward<file_logger>(other))
{}

file_logger& file_logger::operator=(file_logger&& other) noexcept 
{
    basic_logger::operator=(std::forward<file_logger>(other));
    return *this;
}

std::streambuf* file_logger::rdbuf()
{
    return good() ? out().rdbuf() : nullptr;
}