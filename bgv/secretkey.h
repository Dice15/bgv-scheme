#pragma once
#include <Eigen/Dense>
#include <vector>

namespace fheprac
{
	class SecretKey
	{
	public:
		SecretKey();

		SecretKey(std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>>&secretkey);

		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> key(int64_t level) const;

	private:
		std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>> sk_;
	};
}