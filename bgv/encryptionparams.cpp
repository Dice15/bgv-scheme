#include "encryptionparams.h"

namespace fheprac
{
	EncryptionParameters::EncryptionParameters(uint64_t q, uint64_t l, uint64_t next_index) :q_(q), l_(l), next_param_index_(next_index)
	{
		// 균등 분포 [0, q-1] 정수
		std::random_device rd;
		rand_.seed(rd());
		uniform_dist_ = std::uniform_int_distribution<uint64_t>(0, q - 1);
	}

	uint64_t EncryptionParameters::next_param_index() const
	{
		return next_param_index_;
	}

	uint64_t& EncryptionParameters::q()
	{
		return q_;
	}

	uint64_t& EncryptionParameters::l()
	{
		return l_;
	}

	uint64_t EncryptionParameters::value_from_uniform_dist()
	{
		return uniform_dist_(rand_);
	}
}