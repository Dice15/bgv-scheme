#include "polynomial.h"
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
			destination.coeffs_[i] = (coeffs_[i] + other.coeffs_[i]) % mod_;
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
			coeffs_[i] += other.coeffs_[i];
			coeffs_[i] %= mod_;
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
			destination.coeffs_[i] = (coeffs_[i] + negate_coeff(other.coeffs_[i])) % mod_;
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
			coeffs_[i] += negate_coeff(other.coeffs_[i]);
			coeffs_[i] %= mod_;
		}
	}

	Polynomial Polynomial::operator-() const
	{
		Polynomial destination(deg_, mod_);

		for (uint64_t i = 0; i <= deg_; i++)
		{
			destination.coeffs_[i] = negate_coeff(coeffs_[i]);
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
				destination.coeffs_[i + j] += safe_coeff_multiply(coeffs_[i], other.coeffs_[j]) % mod_;
				destination.coeffs_[i + j] %= mod_;
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
				destination.coeffs_[i - red] += negate_coeff(destination.coeffs_[i]);
				destination.coeffs_[i - red] %= mod_;
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
			destination.coeffs_[i] = safe_coeff_multiply(coeffs_[i], other) % mod_;
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
				destination.coeffs_[i + j] += safe_coeff_multiply(coeffs_[i], other.coeffs_[j]) % mod_;
				destination.coeffs_[i + j] %= mod_;
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
				destination.coeffs_[i - red] += negate_coeff(destination.coeffs_[i]);
				destination.coeffs_[i - red] %= mod_;
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
			coeffs_[i] = safe_coeff_multiply(coeffs_[i], other) % mod_;
		}
	}

	uint64_t Polynomial::negate_coeff(uint64_t coeff) const
	{
		return mod_ - (coeff % mod_);
	}

	uint64_t Polynomial::safe_coeff_multiply(uint64_t coeff1, uint64_t coeff2) const 
	{
		uint64_t destination = 0;

		while (coeff2 > 0) 
		{
			// coeff2의 마지막 비트가 1인 경우, destination에 coeff1을 추가
			if (coeff2 & 1) 
			{
				destination = (destination + coeff1) % mod_;
			}

			// coeff1을 2배로 증가시키고 모듈러스 연산 적용
			coeff1 = (coeff1 << 1) % mod_;

			// coeff2의 다음 비트로 이동
			coeff2 >>= 1;
		}

		return destination;
	}
}