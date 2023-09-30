#pragma once

#include <string>
#include <vector>
#include <ostream>

class protocol
{
public:
    protocol(std::string const &title) : _title{title} {}
    template <typename container>
    void set_table_headers(container const &cont)
    {
        _headers.assign(std::begin(cont), std::end(cont));
    }
    // void print(std::ostream &os, std::vector<> const &iterations) const;

private:
    std::string _title;
    std::vector<std::string> _headers;
};