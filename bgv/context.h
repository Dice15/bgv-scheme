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
		bool is_prime(uint64_t n) const;

		uint64_t find_relatively_prime(uint64_t min_val, uint64_t rp_factor) const;

		std::vector<uint64_t> create_modulus_chain(uint64_t t, uint64_t l) const;

		uint64_t deg_;         // modulus degree

		uint64_t plain_mod_;   // plain modulus

		uint64_t dep_;         // depth

		uint64_t n_;           // matrix dimension

		uint64_t N_;

		std::vector<EncryptionParameters> params_;
	};
}