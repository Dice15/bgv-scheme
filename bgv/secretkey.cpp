#include "secretkey.h"

namespace fheprac
{
	SecretKey::SecretKey() {}

	SecretKey::SecretKey(std::vector<PolyMatrix>& poly_matrix) : sk_(poly_matrix) {}

	PolyMatrix& SecretKey::data(int64_t level)
	{
		return sk_[level];
	}
}