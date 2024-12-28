#pragma once

#include <stdint.h>
#include <vector>

namespace fheprac
{
	class Polynomial
	{
	public:
		Polynomial();

		Polynomial(uint64_t degree, uint64_t modulus, uint64_t value = 0);

		uint64_t get(uint64_t index) const;

		std::vector<uint64_t> get() const;

		void set(uint64_t index, uint64_t value);

		void set(std::vector<uint64_t> coeffients);

		uint64_t degree() const;

		uint64_t modulus() const;

		void assign(uint64_t degree, uint64_t modulus, uint64_t value = 0);

		void reset(uint64_t degree, uint64_t modulus, uint64_t value = 0);

		Polynomial operator+(const Polynomial& other) const;

		void operator+=(const Polynomial& other);

		Polynomial operator-(const Polynomial& other) const;

		void operator-=(const Polynomial& other);

		Polynomial operator-() const;

		Polynomial operator*(const Polynomial& other) const;

		Polynomial operator*(const uint64_t& other) const;

		void operator*=(const Polynomial& other);

		void operator*=(const uint64_t& other);

	private:
		uint64_t negate_coeff(uint64_t coeff) const;

		uint64_t safe_coeff_multiply(uint64_t coeff1, uint64_t coeff2) const;

		std::vector<uint64_t> coeffs_;

		uint64_t deg_;

		uint64_t mod_;
	};
}