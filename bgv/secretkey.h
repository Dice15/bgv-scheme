#pragma once

#include "util/polymatrix.h"
#include <vector>

namespace fheprac
{
	class SecretKey
	{
	public:
		SecretKey();

		SecretKey(std::vector<PolyMatrix>& poly_matrix);

		PolyMatrix& data(int64_t level);

	private:
		std::vector<PolyMatrix> sk_;
	};
}