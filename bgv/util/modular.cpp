#include "modular.h"

namespace fheprac
{
	void mod(Eigen::Vector<int64_t, Eigen::Dynamic>& vector, int64_t mod_factor)
	{
		// TODO: x�� ��Ұ� mod_factor���� ũ�� ���� �߻�
		vector = vector.unaryExpr([&mod_factor](int64_t x) { return x % mod_factor; });
	}

	void negate(Eigen::Vector<int64_t, Eigen::Dynamic>& vector, int64_t mod_factor)
	{
		// TODO: x�� ��Ұ� mod_factor���� ũ�� ���� �߻�
		vector = vector.unaryExpr([&mod_factor](int64_t x) { return mod_factor - x; });
	}
}