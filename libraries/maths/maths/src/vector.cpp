#include <dynamic_vector.h>
#include <assertion.h>
#include <cstring>
#include <cmath>
#include <iterator>

namespace math {

    void vector::copy(const vector& other) {
        _size = other._size;
        _elems = new double[_size];
        std::memcpy(_elems, other._elems, _size * sizeof(double));
    }

    void vector::move(vector& other) noexcept {
        std::swap(_size, other._size);
        std::swap(_elems, other._elems);
    }

    vector::vector(size_t size) {
        _size = size;
        _elems = new double[_size]{};
    }

    vector::vector(size_t size, double value) {
        _size = size;
        _elems = new double[_size];
        std::fill(begin(), end(), value);
    }

    vector::vector(std::initializer_list<double> list) {
        _size = list.size();
        _elems = new double[_size]{};
        std::copy(list.begin(), list.end(), begin());
    }

    vector::vector(const vector& other) {
        copy(other);
    }

    vector::vector(vector&& other) noexcept : vector() {
        move(other);
    }

    vector::~vector() {
        if (_elems) {
            delete[] _elems;
            _elems = nullptr;
        }
    }

    vector& vector::operator= (const vector& other) {
        copy(other);
        return *this;
    }

    vector& vector::operator= (vector&& other) noexcept {
        move(other);
        return *this;
    }

    vector& vector::operator+= (const vector& other) {
        ASSERT(_size == other._size, "expected the vectors of the same length");

        for (size_t i{}; i < _size; ++i) _elems[i] += other._elems[i];

        return *this;
    }


    vector& vector::operator-= (const vector& other) {
        ASSERT(_size == other._size, "expected the vectors of the same size");

        for (size_t i{}; i < _size; ++i) _elems[i] -= other._elems[i];

        return *this;
    }

    vector& vector::operator*= (double value) {
        for (size_t i{}; i < _size; ++i) _elems[i] *= value;
        return *this;
    }

    vector& vector::operator/= (double value) {
        for (size_t i{}; i < _size; ++i) _elems[i] /= value;
        return *this;
    }

    double operator* (const vector& left, const vector& right) {
        ASSERT(left._size == right._size, "expected the vectors of the same size");

        double result{};
        for (size_t i{}; i < left._size; ++i) result += left._elems[i] * right._elems[i];

        return result;;
    }

    vector operator+ (const vector& left, const vector& right) {
        return vector(left) += right;
    }

    vector operator- (const vector& left, const vector& right) {
        return vector(left) -= right;
    }

    vector operator* (double value, const vector& vec) {
        return vector(vec) *= value;
    }

    vector operator* (const vector& vec, double value) {
        return vector(vec) *= value;
    }

    vector operator/ (const vector& vec, double value) {
        return vector(vec) /= value;
    }

    std::ostream& operator<< (std::ostream& os, const vector& vec) {
        os << "( ";
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(os, " "));
        return os << ")";
    }

    std::istream& operator>> (std::istream& is, vector& vc) {
        for (size_t i{}; i < vc._size; ++i) is >> vc[i];
        return is;
    }

    double& math::vector::first() {
        ASSERT(_size > 0, "failed to get first element from empty vector");
        return _elems[0];
    }

    const double& math::vector::first() const {
        ASSERT(_size > 0, "failed to get first element from empty vector");
        return _elems[0];
    }

    double& math::vector::last() {
        ASSERT(_size > 0, "failed to get last element from empty vector");
        return _elems[_size - 1];
    }

    const double& math::vector::last() const {
        ASSERT(_size > 0, "failed to get last element from empty vector");
        return _elems[_size - 1];
    }

    double length(const vector& v) {
        return std::sqrt(v * v);
    }

    void normalize(vector& vec) {
        vec /= length(vec);
    }
}
