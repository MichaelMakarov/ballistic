#pragma once

#include <interval.hpp>

#include <optimization.hpp>

#include <ostream>
#include <vector>

struct vec_residual
{
    double i;
    double a;
};

class optimization_logger : public math::iterations_saver
{
public:
    optimization_logger(std::size_t max_iter_count, measuring_interval const &inter);

    void save(math::iteration &&iter) override;

    void print(std::ostream &os) const;

    std::vector<vec_residual> get_first_iteration_residuals() const;
    std::vector<vec_residual> get_last_iteration_residuals() const;

private:
    measuring_interval _interval;
    std::vector<math::iteration> _iterations;
};