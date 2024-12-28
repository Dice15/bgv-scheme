#pragma once

#include "polynomial.h"
#include <stdint.h>
#include <vector>

namespace fheprac
{
	class PolyMatrix
	{
	public:
		PolyMatrix();

		PolyMatrix(uint64_t row_size, uint64_t col_size, uint64_t degree, uint64_t modulus, uint64_t value = 0);

		uint64_t get(uint64_t row, uint64_t col, uint64_t index) const;

		Polynomial get(uint64_t row, uint64_t col) const;

		void set(uint64_t row, uint64_t col, uint64_t index, uint64_t value);

		void set(uint64_t row, uint64_t col, Polynomial poly);

		uint64_t row_size() const;

		uint64_t col_size() const;

		uint64_t degree() const;

		uint64_t modulus() const;

		void assign(uint64_t row_size, uint64_t col_size, uint64_t degree, uint64_t modulus, uint64_t value = 0);

		void reset(uint64_t row_size, uint64_t col_size, uint64_t degree, uint64_t modulus, uint64_t value = 0);

		PolyMatrix t() const;

		PolyMatrix operator+(const PolyMatrix& other) const;

		void operator+=(const PolyMatrix& other);

		PolyMatrix operator-(const PolyMatrix& other) const;

		void operator-=(const PolyMatrix& other);

		PolyMatrix operator-() const;

		PolyMatrix operator*(const PolyMatrix& other) const;

		PolyMatrix operator*(const uint64_t& other) const;

		void operator*=(const PolyMatrix& other);

		void operator*=(const uint64_t& other);

	private:
		std::vector<std::vector<Polynomial>> elems_;

		uint64_t row_size_;

		uint64_t col_size_;

		uint64_t deg_;

		uint64_t mod_;
	};
}