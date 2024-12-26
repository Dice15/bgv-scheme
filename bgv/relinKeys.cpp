#include "relinkeys.h"

namespace fheprac
{
	RelinKeys::RelinKeys() {}

	RelinKeys::RelinKeys(std::vector<PolyMatrix>& poly_matrix) : rk_(poly_matrix) {}

	PolyMatrix& RelinKeys::data(int64_t level)
	{
		return rk_[level];
	}
}