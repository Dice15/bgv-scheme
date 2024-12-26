#pragma once

#include "util/polymatrix.h"
#include <vector>

namespace fheprac
{
	class PublicKey
	{
	public:
		PublicKey();

		PublicKey(std::vector<PolyMatrix>& poly_matrix);

		PolyMatrix& data(int64_t level);

	private:
		std::vector<PolyMatrix> pk_;
	};
}