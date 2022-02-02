#pragma once
#include "mathlib/polynomial.h"
#include "mathlib/arithmetics.h"
#include <array>


namespace ball {

	template<size_t size>
	struct array_view {
		double* data;

		double& operator[] (size_t index) { 
#if defined DEBUG || _DEBUG
			if (size <= index) throw std::runtime_error("index out of range");
#endif
			return data[index]; 
		}
		const double& operator[] (size_t index) const { 
#if defined DEBUG || _DEBUG
			if (size <= index) throw std::runtime_error("index out of range");
#endif
			return data[index]; 
		}
	};

	/// <summary>
	/// Interface that wrapes a model of motion and provides functions for computations and model updating
	/// </summary>
	template<size_t vecdim, size_t modelvars, size_t measvars>
	class model_wrapper {
	public:
		/// <summary>
		/// Performs computations and fills the vector of residuals and matrix of partial derivatives.
		/// </summary>
		/// <param name="reference_mp">a reference parameter of motion</param>
		/// <param name="deviated_mp">the deviated parameters of motion</param>
		virtual void compute(std::vector<array_view<measvars>>& residuals, 	std::vector<std::array<array_view<measvars>, modelvars>>& derivatives) const = 0;
		/// <summary>
		/// Updates the parameters of the model of motion.
		/// </summary>
		/// <param name="param_corrections">an array of correction values</param>
		virtual void update(array_view<modelvars> param_corrections) = 0;
		/// <summary>
		/// A count of used points.
		/// </summary>
		/// <returns></returns>
		virtual size_t points_count() const = 0;
	};

	/// <summary>
	/// Solves the task of model parameters adjustment by least squares method.
	/// </summary>
	/// <param name="model_interface">an interface of model of motion</param>
	/// <param name="logbuf">buffer of logging stream</param>
	/// <param name="eps">a precision to control the iterations</param>
	/// <param name="max_iterations">maximum number of iterations to limit the solving process</param>
	/// <returns>an number of iterations</returns>
	template<size_t vecdim, size_t modelvars, size_t measvars>
	size_t evaluate_params(
		model_wrapper<vecdim, modelvars, measvars>& model_interface,
		std::streambuf* logbuf = nullptr,
		double eps = 1e-3,
		size_t max_iterations = 10)
	{
		size_t iteration{ 1 };
		double prev_sum = std::numeric_limits<double>::max();
		size_t points_count = model_interface.points_count();
		// a vector of residuals
		math::vector vc(measvars * points_count);
		// a matrix of partial derivatives
		math::matrix mx(modelvars, vc.size());
		// an array of vector pointers
		std::vector<array_view<measvars>> vector_ptrs(points_count);
		// an array of matrix pointers
		std::vector<std::array<array_view<measvars>, modelvars>> matrix_ptrs(points_count);
		// output stream
		std::ostream logout{ logbuf };
		// a number of symbols of separate line
		constexpr size_t separate_line_width{ 50 };
		constexpr char separate_paragraph_symbol{ '*' };
		constexpr char separate_section_symbol{ '-' };

		auto print_separate_line = [separate_line_width](std::ostream& str, char symbol) {
			str << std::endl;
			for (size_t i{} i < separate_line_width; ++i) str << symbol;
			str << std::endl;
		};

		// initializing pointers of vector and matrix
		for (size_t point_index{}; point_index < points_count; ++point_index) {
			size_t index{ point_index * measvars };
			vector_ptrs[point_index].data = &vc[index];
			for (size_t deriv_index{}; deriv_index < modelvars; ++deriv_index) {
				matrix_ptrs[point_index][deriv_index].data = &mx(deriv_index, index);
			}
		}
 
		/*logout << separate_paragraph_line << std::endl << "AMOUNT OF MEASUREMENTS" << std::endl;
		for (auto it = measurements_begin; it != measurements_end; ++it, ++index) {
			logout << index + 1 << ") " << *it << std::endl;
		}*/
		print_separate_line(logout, separate_paragraph_symbol);
		logout << "ITERATIONS OF SOLUTION";

		for (iteration = 1; iteration <= max_iterations && ; ++iteration) {
			print_separate_line(logout, separate_section_symbol);
			logout << "iteration ¹ " << iteration << std::endl;
			
			model_interface.compute(vector_ptrs, matrix_ptrs);
			auto delta_values = math::lstsq(mx, vc);
			model_interface.update_model(array_view<modelvars>{ delta_values.data() });
			double curr_sum = vc.length();

			logout << "computed corrections of parameters: ";
			std::copy(std::begin(delta_values), std::end(delta_values), std::ostream_iterator<double>(logout, " "));
			logout << std::endl << "function of residuals has value " << curr_sum << std::endl;

			if (math::is_equal(prev_sum, curr_sum, eps)) break;
			prev_sum = curr_sum;
		}

		return iteration;
	}

}