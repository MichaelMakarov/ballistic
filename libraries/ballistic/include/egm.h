#pragma once
#include <structures.h>
#include <vector>
#include <istream>

namespace ball {
	/// <summary>
	/// Potential harmonic of potential decomposition
	/// </summary>
	struct potential_harmonic { double cos, sin; };

	enum struct egm_type : unsigned {
		JGM3, EGM96, EGM08
	};

	template<egm_type>
	struct egm_constants {
		constexpr static double mu{};
		constexpr static double rad{};
		constexpr static double ecsqr{};
		constexpr static double angv{};
		constexpr static double flat{};
		constexpr static size_t count{};
		static std::vector<potential_harmonic> harmonics;
	};

	template<egm_type type>
	void load_harmonics_for(std::istream& instr) {
		using egmc_t = egm_constants<type>;
		egmc_t::harmonics.resize((egmc_t::count + 1) * (egmc_t::count + 2) / 2);
		std::copy(std::istream_iterator<potential_harmonic>{ instr }, {}, egmc_t::harmonics.begin());
	}
	template<egm_type type>
	void load_harmonics_for(std::istream&& instr) {
		load_harmonics_for<type>(instr);
	}
	
	/// <summary>
	/// Computes the height above the ellipsoid using coordinates in GCS.
	/// </summary>
	/// <param name="x">x coordinate [m]</param>
	/// <param name="y">y coordinate [m]</param>
	/// <param name="z">z coordinate [m]</param>
	/// <param name="rad">equatorial radius [m]</param>
	/// <param name="flat">polar flattening</param>
	/// <returns>>height [m]</returns>
	double height_above_ellipsoid(double x, double y, double z, double rad, double flat);

	std::istream& operator>> (std::istream& is, potential_harmonic& ph);

	/// <summary>
	/// Computes mean sidereal time.
	/// </summary>
	/// <param name="jd"></param>
	/// <returns>radians</returns>
	double sidereal_time_mean(const JD& jd);
	/// <summary>
	/// Computes true sidereal time.
	/// </summary>
	/// <param name="jd"></param>
	/// <returns>radians</returns>
	double sidereal_time_true(const JD& jd);

	template<>
	struct egm_constants<egm_type::JGM3> {
		constexpr static double mu{ 0.3986004415E+15 };
		constexpr static double rad{ 0.6378136300E+07 };
		constexpr static double ecsqr{ 6.69437999014e-3 };
		constexpr static double angv{ 72.92115e-6 };
		constexpr static double flat{ 1.0 / 298.257223563 };
		constexpr static size_t count{ 70 };
		static std::vector<potential_harmonic> harmonics;

		static void load_harmonics(std::istream& instr) {
			load_harmonics_for<egm_type::JGM3>(instr);
		}
		void load_harmonics(std::istream&& instr) {
			load_harmonics_for<egm_type::JGM3>(std::forward<std::istream>(instr));
		}
	};

	template<>
	struct egm_constants<egm_type::EGM96> {
		constexpr static double mu{ 0.3986004415E+15 };
		constexpr static double rad{ 0.6378136300E+07 };
		constexpr static double ecsqr{ 6.69437999014e-3 };
		constexpr static double angv{ 72.92115e-6 };
		constexpr static double flat{ 1.0 / 298.257223563 };
		constexpr static size_t count{ 360 };
		static std::vector<potential_harmonic> harmonics;

		static void load_harmonics(std::istream& instr) {
			load_harmonics_for<egm_type::EGM96>(instr);
		}
		static void load_harmonics(std::istream&& instr) {
			load_harmonics_for<egm_type::EGM96>(std::forward<std::istream>(instr));
		}
	};

	template<>
	struct egm_constants<egm_type::EGM08> {
		constexpr static double mu{ 0.3986004415E+15 };
		constexpr static double rad{ 0.6378136300E+07 };
		constexpr static double ecsqr{ 6.69437999014e-3 };
		constexpr static double angv{ 72.92115e-6 };
		constexpr static double flat{ 1.0 / 298.257223563 };
		constexpr static size_t count{ 2190 };
		static std::vector<potential_harmonic> harmonics;

		static void load_harmonics(std::istream& instr) {
			load_harmonics_for<egm_type::EGM08>(instr);
		}
		void load_harmonics(std::istream&& instr) {
			load_harmonics_for<egm_type::EGM08>(std::forward<std::istream>(instr));
		}
	};

	using JGM3 = egm_constants<egm_type::JGM3>;
	using EGM96 = egm_constants<egm_type::EGM96>;
	using EGM08 = egm_constants<egm_type::EGM08>;
}