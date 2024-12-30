#pragma once
#include "encryptionparams.h"
#include <stdint.h>
#include <vector>

namespace fheprac
{
	class Context
	{
	public:
		Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t depth);

		uint64_t poly_modulus_degree() const;

		uint64_t plain_modulus_value() const;

		uint64_t depth() const;

		EncryptionParameters first_param() const;

		EncryptionParameters last_param() const;

		EncryptionParameters param(uint64_t index) const;

	private:
		uint64_t get_prime(const uint64_t factor, const int bit_size) const;

		uint64_t get_relatively_prime_odd(uint64_t min_val, uint64_t rp_factor) const;

		std::vector<uint64_t> create_modulus_chain() const;

		uint64_t deg_;         // modulus degree

		uint64_t plain_mod_;   // plain modulus

		uint64_t dep_;         // depth

		uint64_t n_;           // matrix dimension

		uint64_t N_;

		std::vector<EncryptionParameters> params_;
	};
}