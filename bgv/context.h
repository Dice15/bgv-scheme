#pragma once
#include "encryptionparams.h"
#include <stdint.h>
#include <random>

namespace fheprac
{
	class Context
	{
	public:
		Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t level);

		uint64_t poly_modulus_degree() const;

		uint64_t plain_modulus_value() const;

		uint64_t level() const;

		uint64_t value_from_gaussian_dist();

		EncryptionParameters first_param() const;

		EncryptionParameters last_param() const;

		EncryptionParameters param(uint64_t index) const;

	private:
		bool is_prime(uint64_t n) const;

		uint64_t find_relatively_prime(uint64_t min_val, uint64_t rp_factor) const;

		std::vector<uint64_t> create_modulus_chain(uint64_t t, uint64_t l) const;

		uint64_t d_;   // modulus degree

		uint64_t t_;   // plain modulus

		uint64_t l_;   // level

		uint64_t n_;

		uint64_t N_;

		std::mt19937 rand_;

		std::normal_distribution<double_t> gaussian_dist_;

		std::vector<EncryptionParameters> params_;
	};
}