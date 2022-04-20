#include <linalg.h>

namespace math {

	vector lstsq(const matrix& mx, const vector& vc) {
		auto sysm = mx * transpose(mx);
		size_t rows = sysm.rows();
		vector diag(rows);
		for (size_t i{}; i < rows; ++i) diag[i] = 1 / std::sqrt(sysm(i, i)); 
		mxd(sysm, diag);
		dxm(diag, sysm);
		inverse(sysm);
		mxd(sysm, diag);
		dxm(diag, sysm);
		return sysm * (mx * vc);
	}

}