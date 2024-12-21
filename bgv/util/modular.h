#pragma once

#include <Eigen/Dense>
#include <stdint.h>

namespace fheprac
{
	void mod(Eigen::Vector<int64_t, Eigen::Dynamic>& vector, int64_t mod_factor);

	void negate(Eigen::Vector<int64_t, Eigen::Dynamic>& vector, int64_t mod_factor);
}