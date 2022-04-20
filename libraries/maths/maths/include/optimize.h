#pragma once
#include <linalg.h>
#include <vector>
#include <array>

namespace math {
	template<size_t size>
	struct array_view {
		double* arr;

		const double& operator[](size_t i) const {
#if defined _DEBUG || DEBUG
			ASSERT(i < size, "index out of bounds");
#endif
			return arr[i];
		}
		double& operator[](size_t i) {
#if defined _DEBUG || DEBUG
			ASSERT(i < size, "index out of bounds");
#endif
			return arr[i];
		}
	};

	/// <summary>
	/// An interface of computations required by newton-raphson optimization
	/// </summary>
	template<size_t params, size_t pointdim>
	struct data_interface {
		/// <summary>
		/// A number of reference values.
		/// </summary>
		virtual size_t points_count() const = 0;
		/// <summary>
		/// Fills the vector of residuals and matrix of partial derivatives.
		/// </summary>
		/// <param name="residuals">an array of pointers to vector of residuals</param>
		/// <param name="derivatives">an array of pointers to matrix of partial derivatives</param>
		virtual void compute(std::vector<array_view<pointdim>>& residuals, std::vector<std::array<array_view<pointdim>, params>>& derivatives) const = 0;
		/// <summary>
		/// Updates the required parameters of computation.
		/// </summary>
		/// <param name="param_corrections">the values to add</param>
		virtual void update(array_view<params> param_corrections) = 0;
	};

	/// <summary>
	/// Newton-Ruphson nonlinear optimization.
	/// </summary>
	/// <param name="interface">an interface that represents the task</param>
	/// <param name="logbuf">a buffer of output stream</param>
	/// <param name="eps">a precision to compare the values of function of residuals</param>
	/// <param name="max_iterations">a max number of iterations to perform</param>
	/// <returns>a number of performed iterations</returns>
	template<size_t params, size_t pointdim>
	size_t newton_raphson(
		data_interface<params, pointdim>& interface, 
		std::streambuf* logbuf, 
		double eps = 1e-3, 
		size_t max_iterations = 10
	)
	{
		size_t iteration{ 1 };
		double prevsum = std::numeric_limits<double>::max();
		size_t points_count = interface.points_count();
		vector vc(pointdim * points_count);
		matrix mx(params, vc.size());
		std::vector<array_view<pointdim>> vector_ptrs(points_count);
		std::vector<std::array<array_view<pointdim>, params>> matrix_ptrs(points_count);

		for (size_t point_index{}; point_index < points_count; ++point_index) {
			size_t vec_index = point_index * pointdim;
			vector_ptrs[point_index].arr = &vc[vec_index];
			for (size_t deriv_index{}; deriv_index < params; ++deriv_index) {
				matrix_ptrs[point_index][deriv_index].arr = &mx(deriv_index, vec_index);
			}
		}

		std::ostream logout{ logbuf };
		constexpr size_t separate_line_width{ 70 };
		constexpr char separate_paragraph_symbol{ '*' };
		constexpr char separate_section_symbol{ '-' };
		auto print_separate_line = [separate_line_width, &logout](char symbol) {
			logout << std::endl;
			for (size_t i{}; i < separate_line_width; ++i) logout << symbol;
			logout << std::endl;
		};

		print_separate_line(separate_paragraph_symbol);
		logout << "ITERATIONS OF SOLUTION";
		for (; iteration <= max_iterations; ++iteration) {
			print_separate_line(separate_section_symbol);
			logout << "iteration ¹ " << iteration << std::endl;

			interface.compute(vector_ptrs, matrix_ptrs);
			auto delta_values = lstsq(mx, vc);

			interface.update(array_view<params>{ delta_values.data() });
			double currsum = length(vc);

			logout << "computed corrections of parameters: ";
			std::copy(
				std::begin(delta_values), std::end(delta_values),
				std::ostream_iterator<double>{ logout, " " }
			);
			logout << std::endl << "function of residuals " << currsum << std::endl;

			if (is_equal(prevsum, currsum, eps)) break;
			prevsum = currsum;
		}

		return std::min(iteration, max_iterations);
	}
}