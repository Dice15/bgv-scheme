#include "publickey.h"

namespace fheprac
{
	PublicKey::PublicKey() {}

	PublicKey::PublicKey(std::vector<PolyMatrix>& poly_matrix) : pk_(poly_matrix) {}

	PolyMatrix& PublicKey::data(int64_t level)
	{
		return pk_[level];
	}
}