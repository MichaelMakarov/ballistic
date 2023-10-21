#pragma once

#include <interval.hpp>
#include <residuals_provider.hpp>

#include <optimization.hpp>

#include <ostream>
#include <vector>


class optimization_logger : public math::iterations_saver, public residuals_provider
{
public:
    optimization_logger(std::size_t max_iter_count, measuring_interval const &inter);

    void save(math::iteration &&iter) override;

    void print(std::ostream &os) const;

    std::vector<residual_point> get_first_iteration_residuals() const override;
    
    std::vector<residual_point> get_last_iteration_residuals() const override;

private:
    measuring_interval _interval;
    std::vector<math::iteration> _iterations;
};