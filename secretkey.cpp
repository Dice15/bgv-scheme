#include "secretkey.h"

namespace fheprac
{
	SecretKey::SecretKey() {}

	SecretKey::SecretKey(std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>>& secretkey) : sk_(secretkey) {}

	std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> SecretKey::key(int64_t level) const
	{
		return sk_[level];
	}
}