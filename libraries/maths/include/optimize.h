#pragma once
#include <linalg.h>
#include <arithmetics.h>
#include <assertion.h>
    
#include <vector>
#include <array>

template<size_t _size>
struct array_view {
	double* ptr;

	const double& operator[](size_t i) const {
		#if defined _DEBUG || DEBUG
		ASSERT(i < _size, "индекс вне диапазона");
		#endif
		return ptr[i];
	}
	double& operator[](size_t i) {
		#if defined _DEBUG || DEBUG
		ASSERT(i < _size, "индекс вне диапазона");
		#endif
		return ptr[i];
	}
};

/**
 * @brief Интерфейс для функции оптимизации
 *
 * @tparam param кол-во параметров оптимизации (для вычисления частных производных)
 * @tparam dim кол-во параметров невязкки для одной точки
 */
template<size_t param, size_t dim>
struct optimize_interface {
	/**
	 * @brief Кол-во точек
	 *
	 * @return size_t
	 */
	virtual size_t points_count() const = 0;
	/**
	 * @brief Вычисление невязок и частных производных
	 *
	 * @param residuals невзяки
	 * @param derivatives частные производные
	 */
	virtual void compute(std::vector<array_view<dim>>& residuals, std::vector<std::array<array_view<dim>, param>>& derivatives) const = 0;
	/**
	 * @brief Коррекция
	 *
	 * @param corrections вектор корректирующих значений
	 */
	virtual void update(array_view<param> corrections) = 0;
};

class wrapping_logger : public basic_logger {
public:
	wrapping_logger(std::streambuf* const buf);
};

template<size_t param, size_t dim>
size_t newton(optimize_interface<param, dim>& interface, std::streambuf* logbuf, double eps = 1e-3, size_t maxiter = 10)
{
	size_t iteration{ 1 };
	double prev_resid = std::numeric_limits<double>::max();
	size_t count = interface.points_count();
	vector vc(dim * count);
	matrix mx(param, vc.size());
	std::vector<array_view<dim>> vcptr(count);
	std::vector<std::array<array_view<dim>, param>> mxptr(count);

	for (size_t point_index{}; point_index < count; ++point_index) {
		size_t vec_index = point_index * dim;
		vcptr[point_index].ptr = &vc[vec_index];
		for (size_t deriv_index{}; deriv_index < param; ++deriv_index) {
			mxptr[point_index][deriv_index].ptr = &mx(deriv_index, vec_index);
		}
	}

	wrapping_logger logger(logbuf);

	logger << "==================\n";
	logger << "|    РЕШЕНИЕ     |\n";
	logger << "==================\n";

	for ( ; iteration <= maxiter; ++iteration) {
		logger << "Итерация № " << iteration << '\n';

		interface.compute(vcptr, mxptr);
		auto delta = lstsq(mx, vc);
		double curr_resid = length(vc);

		interface.update(array_view<param>{ delta.data() });

		logger << "Ф-ция невязок = " << curr_resid << std::endl;
		logger << "Коррекция параметров:\n";
		for (size_t i{}; i < delta.size(); ++i) {
			logger << "параметр " << i + 1 << " = " << delta[i] << std::endl;
		}
		// logger << "Невязки:\n";
		// for (size_t m{}; m < count; ++m) {
		// 	for (size_t i{}; i < dim; ++i) {
		// 		logger << vcptr[m][i] << ' ';
		// 	}
		// 	logger << std::endl;
		// }
		// logger << "Частные производные:\n";
		// for (size_t r{}; r < count; ++r) {
		// 	for (size_t c{}; c < param; ++c) {
		// 		logger << "( ";
		// 		for (size_t i{}; i < dim; ++i) {
		// 			logger << mxptr[r][c][i] << ' ';
		// 		}
		// 		logger << ") ";
		// 	}
		// 	logger << std::endl;
		// }
		logger << "----------------------------------------------\n";

		if (is_equal(prev_resid, curr_resid, eps)) break;
		prev_resid = curr_resid;
	}

	return std::min(iteration, maxiter);
}

