#pragma once
#include <juliandate.h>

namespace ball {
	/// <summary>
	/// Solar activity parameters
	/// </summary>
	struct dma_params {
		double f10_7, f81, kp;
	};

	using JD = times::juliandate;

	/// <summary>
	/// A model of atmosphere according GOST2004
	/// </summary>
	struct atmosphere2004 {
		/// <summary>
		/// Computes density of atmosphere.
		/// </summary>
		/// <param name="pos">is point to compute at (in GCS)</param>
		/// <param name="jd"is julian date></param>
		/// <param name="solar_pos">is solar positon in GCS</param>
		/// <param name="solar_inclination">is solar inclination</param>
		/// <returns></returns>
		double density(double h, double x, double y, double z, const JD& jd, double sunlong, double sunincl, const dma_params& dma) const;
	};
}