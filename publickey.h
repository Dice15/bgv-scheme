#pragma once
#include <Eigen/Dense>
#include <vector>

namespace fheprac
{
	class PublicKey
	{
	public:
		PublicKey();

		PublicKey(std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>>& publickey);

		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> key(int64_t level) const;

	private:
		std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>> pk_;
	};
}