#pragma once

#include "util/polymatrix.h"
#include <vector>

namespace fheprac
{
	class RelinKeys
	{
	public:
		RelinKeys();

		RelinKeys(std::vector<PolyMatrix>& poly_matrix);

		PolyMatrix& data(int64_t level);  // T_(s_l -> s_(l-1))

	private:
		std::vector<PolyMatrix> rk_;
	};
}