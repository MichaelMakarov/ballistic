#pragma once

#include <string>

class table_data_provider {
  public:
    virtual ~table_data_provider() = default;

    virtual std::string get_data(std::size_t row, std::size_t column) const = 0;

    virtual int get_alignment(std::size_t column) const = 0;

    virtual std::string get_header(std::size_t column) const = 0;

    virtual std::size_t get_rows_count() const = 0;

    virtual std::size_t get_columns_count() const = 0;
};
