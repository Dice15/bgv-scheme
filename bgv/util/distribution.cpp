#include "distribution.h"
#include "safeoperation.h"
#include <random>

namespace fheprac
{
	Polynomial sample_poly_from_uniform_dist(const uint64_t poly_modulus_degree, const uint64_t modulus)
	{
		uint64_t min = static_cast<uint64_t>(0);
		uint64_t max = modulus - static_cast<uint64_t>(1);

		// [0, q-1] ���� �յ� ���� ����.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::uniform_int_distribution<uint64_t> dist(min, max);

		// R = Z[x] / x^d + 1
		Polynomial destination(poly_modulus_degree, modulus);

		for (uint64_t i = 0; i < poly_modulus_degree; i++)
		{
			destination.set(i, dist(rand));
		}

		return destination;
	}

	Polynomial sample_poly_from_gaussian_dist(const uint64_t poly_modulus_degree, const uint64_t modulus)
	{
		double_t E = 0.0;
		double_t SD = 3.2;

		// ǥ�������� 3.2�� ����þ� ������ ����.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::normal_distribution<double_t> dist(E, SD);

		// R = Z[x] / x^d + 1
		Polynomial destination(poly_modulus_degree, modulus);

		for (uint64_t i = 0; i < poly_modulus_degree; i++)
		{
			double_t r = dist(rand);
			uint64_t v = mod(static_cast<uint64_t>(std::abs(std::llround(r))), modulus);

			if (std::signbit(r))
			{
				v = modulus - v;
			}

			destination.set(i, v);
		}

		return destination;
	}

	Polynomial sample_poly_from_hamming_dist(const uint64_t poly_modulus_degree, const uint64_t modulus)
	{
		uint64_t h = std::min(static_cast<uint64_t>(64), poly_modulus_degree);   // CRT�� �����ϱ� �������� ���� ������ ���׽��� ������� ���ϹǷ� �ӽ÷� h�� ����.

		// -1, +1 �߿��� �����ϵ��� �յ� ���� ����. (0�� -1 ���).
		std::random_device rd;
		std::mt19937 rand(rd());
		std::uniform_int_distribution<uint64_t> dist(0, 1);

		// ���׽� ��� �ε����� �����ϰ� ����.
		std::vector<uint64_t> index(poly_modulus_degree);
		std::iota(index.begin(), index.end(), 0);   // [0, 1, ..., poly_degree-1]
		std::shuffle(index.begin(), index.end(), rand);

		// R = Z[x] / x^d + 1
		Polynomial destination(poly_modulus_degree, modulus);

		for (uint64_t i = 0; i < h; i++)
		{
			uint64_t r = dist(rand);
			uint64_t v = r;

			if (r == static_cast<uint64_t>(0))
			{
				v = modulus - static_cast<uint64_t>(1);
			}

			destination.set(index[i], v);
		}

		return destination;
	}

	Polynomial sample_poly_from_zero_one_dist(const uint64_t poly_modulus_degree, const uint64_t modulus)
	{
		double_t pr = 0.5;

		// [0.0, 1.0] �Ǽ� �յ� ���� ����.
		std::random_device rd;
		std::mt19937 rand(rd());
		std::uniform_real_distribution<double_t> dist(0.0, 1.0);

		// R = Z[x] / x^d + 1
		Polynomial destination(poly_modulus_degree, modulus);

		for (uint64_t i = 0; i < poly_modulus_degree; i++)
		{
			uint64_t r = dist(rand);
			uint64_t v = 0;

			if (r < 0.25)
			{
				v = modulus - static_cast<uint64_t>(1);
			}
			else if (r < 0.5)
			{
				v = static_cast<uint64_t>(1);
			}
			else
			{
				v = static_cast<uint64_t>(0);
			}


			destination.set(i, v);
		}

		return destination;
	}
}