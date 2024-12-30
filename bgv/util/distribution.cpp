#include "distribution.h"
#include <random>

namespace fheprac
{
	uint64_t sample_from_gaussian_dist(const Context& context, const EncryptionParameters& params)
	{
		uint64_t q = params.q();

		// 표준편차가 3.2인 가우시안 분포로 설정.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::normal_distribution<double_t> dist(0.0, 3.2);

		double_t r = dist(rand);
		uint64_t v = static_cast<uint64_t>(std::abs(std::llround(r))) % q;

		if (std::signbit(r))
		{
			v = q - v;
		}

		return v;
	}

	Polynomial sample_poly_from_gaussian_dist(const Context& context, const EncryptionParameters& params)
	{
		uint64_t q = params.q();

		// 표준편차가 3.2인 가우시안 분포로 설정.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::normal_distribution<double_t> dist(0.0, 3.2);

		// R = Z[x] / x^d + 1
		Polynomial destination(context.poly_modulus_degree() - static_cast<uint64_t>(1), q);

		for (uint64_t i = 0; i <= destination.degree(); i++)
		{
			double_t r = dist(rand);
			uint64_t v = static_cast<uint64_t>(std::abs(std::llround(r))) % q;

			if (std::signbit(r))
			{
				v = q - v;
			}

			destination.set(i, v);
		}

		return destination;
	}

	uint64_t sample_from_uniform_dist(const Context& context, const EncryptionParameters& params)
	{
		uint64_t q = params.q();

		// 균등 분포 [0, q-1] 정수.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::uniform_int_distribution<uint64_t> dist(0, q - uint64_t(1));

		return dist(rand);
	}

	Polynomial sample_poly_from_uniform_dist(const Context& context, const EncryptionParameters& params)
	{
		uint64_t q = params.q();

		// 균등 분포 [0, q-1] 정수.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::uniform_int_distribution<uint64_t> dist(0, q - uint64_t(1));

		// R = Z[x] / x^d + 1
		Polynomial destination(context.poly_modulus_degree() - static_cast<uint64_t>(1), q);

		for (uint64_t i = 0; i <= destination.degree(); i++)
		{
			destination.set(i, dist(rand));
		}

		return destination;
	}
}