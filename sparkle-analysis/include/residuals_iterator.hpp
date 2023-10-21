#pragma once

#include <residuals_provider.hpp>

template <double residual_point::*field>
    class residuals_iterator {
        residual_point *_ptr;

      public:
        residuals_iterator(residual_point *ptr)
            : _ptr{ptr} {
        }

        residuals_iterator &operator++() {
            ++_ptr;
            return *this;
        }

        bool operator!=(residuals_iterator other) const {
            return _ptr != other._ptr;
        }

        double& operator*() const {
            return _ptr->*field;
        }
    };
