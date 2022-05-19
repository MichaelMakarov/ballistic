#pragma once
#include <cmath>

/**
 * @brief Статистические данные
 *
 */
struct statoutput {
	/**
	 * @brief МО
	 *
	 */
	double mean;
	/**
	 * @brief СКО
	 *
	 */
	double std;
};
/**
 * @brief Вычисление МО и СКО
 *
 * @tparam fwdit итератор последовательного доступа
 * @param begin итератор на начало коллекции
 * @param end итератор на конец коллекции
 * @return статистические данные
 */
template<typename fwdit>
constexpr statoutput mean_std(fwdit begin, fwdit end) {
	statoutput out{};
	size_t count{};

	for ( ; begin != end; ++begin, ++count) {
		double tmp = *begin;
		out.mean += tmp;
		out.std += tmp * tmp;
	}
	out.mean /= count;
	out.std = std::sqrt(out.std / count - out.mean * out.mean);

	return out;
}
