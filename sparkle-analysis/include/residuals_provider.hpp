#pragma once

#include <vector>

struct residual_point {
    double i;
    double a;
};

class residuals_provider {
  public:
    virtual ~residuals_provider() = default;

    virtual std::vector<residual_point> get_first_iteration_residuals() const = 0;

    virtual std::vector<residual_point> get_last_iteration_residuals() const = 0;
};