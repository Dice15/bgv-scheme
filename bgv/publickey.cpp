#include "publickey.h"

namespace fheprac
{
	PublicKey::PublicKey() {}

	PublicKey::PublicKey(std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>>& publickey) : pk_(publickey) {}

	std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> PublicKey::key(int64_t level) const
	{
		return pk_[level];
	}
}