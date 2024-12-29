#pragma once
#include <stdint.h>

namespace fheprac
{
	class EncryptionParameters
	{
	public:
		EncryptionParameters() = default;

		EncryptionParameters(uint64_t q, uint64_t l, uint64_t next_index);

		uint64_t& q();

		const uint64_t& q() const;

		uint64_t& l();

		const uint64_t& l() const;

		uint64_t next_param_index() const;

	private:
		uint64_t q_;

		uint64_t l_;   // level

		uint64_t next_param_index_;
	};
}