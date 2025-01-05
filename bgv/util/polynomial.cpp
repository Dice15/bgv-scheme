#include "polynomial.h"
#include "safeoperation.h"
#include <stdexcept>
#include <iostream>

namespace fheprac
{
	Polynomial::Polynomial() :deg_(0), mod_(0) {}

	Polynomial::Polynomial(const uint64_t degree, const uint64_t modulus, const uint64_t value) :deg_(degree), mod_(modulus)
	{
		coeffs_.assign(deg_ + 1, value);
	}

	uint64_t Polynomial::get(const size_t index) const
	{
		if (index >= coeffs_.size())
		{
			throw std::out_of_range("Index out of range.");
		}

		return coeffs_[index];
	}

	const std::vector<uint64_t>& Polynomial::get() const
	{
		return coeffs_;
	}

	void Polynomial::set(const size_t index, const uint64_t value)
	{
		if (index >= coeffs_.size())
		{
			throw std::out_of_range("Index out of range.");
		}

		coeffs_[index] = value % mod_;
	}

	void Polynomial::set(const std::vector<uint64_t>& coeffients)
	{
		for (size_t i = 0; i < coeffients.size(); i++)
		{
			set(i, coeffients[i]);
		}
	}

	uint64_t Polynomial::degree() const
	{
		return deg_;
	}

	uint64_t Polynomial::modulus() const
	{
		return mod_;
	}

	void Polynomial::assign(const uint64_t degree, const uint64_t modulus, const uint64_t value)
	{
		deg_ = degree;
		mod_ = modulus;
		coeffs_.assign(deg_ + 1, value);
	}

	void Polynomial::reset(const uint64_t degree, const uint64_t modulus, const uint64_t value)
	{
		uint64_t mod_prev = mod_;
		uint64_t mod_prev_h = mod_prev >> 1;
		uint64_t mod_curr = modulus;

		deg_ = degree;
		mod_ = modulus;
		coeffs_.resize(deg_ + 1, value);

		for (size_t i = 0; i <= deg_; i++)
		{
			uint64_t coeff = coeffs_[i];

			if (coeff > mod_prev_h)
			{
				coeff = mod_curr - ((mod_prev - coeff) % mod_curr);
			}
			else
			{
				coeff = coeff % mod_curr;
			}

			coeffs_[i] = coeff;
		}
	}

	Polynomial Polynomial::operator+(const Polynomial& other) const
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		Polynomial destination(deg_, mod_);

		for (uint64_t i = 0; i <= deg_; i++)
		{
			destination.coeffs_[i] = add_mod_safe(coeffs_[i], other.coeffs_[i], mod_);
		}

		return destination;
	}

	void Polynomial::operator+=(const Polynomial& other)
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		for (uint64_t i = 0; i <= deg_; i++)
		{
			coeffs_[i] = add_mod_safe(coeffs_[i], other.coeffs_[i], mod_);
		}
	}

	Polynomial Polynomial::operator-(const Polynomial& other) const
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		Polynomial destination(deg_, mod_);

		for (uint64_t i = 0; i <= deg_; i++)
		{
			destination.coeffs_[i] = add_mod_safe(coeffs_[i], negate_mod_safe(other.coeffs_[i], mod_), mod_);
		}

		return destination;
	}

	void Polynomial::operator-=(const Polynomial& other)
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		for (uint64_t i = 0; i <= deg_; i++)
		{
			coeffs_[i] = add_mod_safe(coeffs_[i], negate_mod_safe(other.coeffs_[i], mod_), mod_);
		}
	}

	Polynomial Polynomial::operator-() const
	{
		Polynomial destination(deg_, mod_);

		for (uint64_t i = 0; i <= deg_; i++)
		{
			destination.coeffs_[i] = negate_mod_safe(coeffs_[i], mod_);
		}

		return destination;
	}


	Polynomial Polynomial::operator*(const Polynomial& other) const
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		Polynomial destination(static_cast<uint64_t>(2) * deg_, mod_);

		for (uint64_t i = 0; i <= deg_; ++i)
		{
			for (uint64_t j = 0; j <= deg_; ++j)
			{
				destination.coeffs_[i + j] = add_mod_safe(destination.coeffs_[i + j], mul_mod_safe(coeffs_[i], other.coeffs_[j], mod_), mod_);
			}
		}

		uint64_t red = deg_ + 1;

		for (uint64_t i = red; i <= destination.deg_; i++)
		{
			if (destination.coeffs_[i] != 0)
			{
				// R = Z[x] / x^d + 1
				// x^i = x^red * x^(i-red) = -1 * x^(i-red)
				// -1 * x^(i-red) + x^(i-red)
				destination.coeffs_[i - red] = add_mod_safe(destination.coeffs_[i - red], negate_mod_safe(destination.coeffs_[i], mod_), mod_);
				destination.coeffs_[i] = 0;
			}
		}

		destination.deg_ = deg_;
		destination.coeffs_.resize(deg_ + 1);

		return destination;
	}

	Polynomial Polynomial::operator*(const uint64_t& other) const
	{
		Polynomial destination(deg_, mod_);

		for (uint64_t i = 0; i <= deg_; i++)
		{
			destination.coeffs_[i] = mul_mod_safe(coeffs_[i], other, mod_);
		}

		return destination;
	}

	void Polynomial::operator*=(const Polynomial& other)
	{
		if (deg_ != other.deg_ || mod_ != other.mod_)
		{
			throw std::invalid_argument("Polynomials must have matching degree, modulus and reduction polynomial.");
		}

		Polynomial destination(static_cast<uint64_t>(2) * deg_, mod_);

		for (uint64_t i = 0; i <= deg_; ++i)
		{
			for (uint64_t j = 0; j <= deg_; ++j)
			{
				destination.coeffs_[i + j] = add_mod_safe(destination.coeffs_[i + j], mul_mod_safe(coeffs_[i], other.coeffs_[j], mod_), mod_);
			}
		}

		uint64_t red = deg_ + 1;

		for (uint64_t i = red; i <= destination.deg_; i++)
		{
			if (destination.coeffs_[i] != 0)
			{
				// R = Z[x] / x^d + 1
				// x^i = x^red * x^(i-red) = -1 * x^(i-red)
				// -1 * x^(i-red) + x^(i-red)
				destination.coeffs_[i - red] = add_mod_safe(destination.coeffs_[i - red], negate_mod_safe(destination.coeffs_[i], mod_), mod_);
				destination.coeffs_[i] = 0;
			}
		}

		for (uint64_t i = 0; i <= deg_; ++i)
		{
			coeffs_[i] = destination.coeffs_[i];
		}
	}

	void Polynomial::operator*=(const uint64_t& other)
	{
		for (uint64_t i = 0; i <= deg_; i++)
		{
			coeffs_[i] = mul_mod_safe(coeffs_[i], other, mod_);
		}
	}
}