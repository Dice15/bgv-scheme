#pragma once
#include <stdint.h>
#include <random>

namespace fheprac
{
	class EncryptionParameters
	{
	public:
		EncryptionParameters() = default;

		EncryptionParameters(uint64_t q, uint64_t l, uint64_t next_index);

		uint64_t next_param_index() const;

		uint64_t& q();

		uint64_t& l();

		uint64_t value_from_uniform_dist();

	private:
		uint64_t next_param_index_;

		uint64_t q_;

		uint64_t l_;   // level

		std::mt19937 rand_;

		std::uniform_int_distribution<uint64_t> uniform_dist_;
	};
}