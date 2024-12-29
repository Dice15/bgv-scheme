#include "keygenerator.h"
#include "util/distribution.h"
#include "util/polynomial.h"
#include "util/polymatrix.h"
#include <vector>
#include <iostream>

namespace fheprac
{
	KeyGenerator::KeyGenerator(const Context& context) : context_(context)
	{
		const uint64_t dep = context_.depth();
		const uint64_t d = context_.poly_modulus_degree();

		PolyMatrix sk;
		sk_.assign(dep + 1);

		// 하나의 비밀키를 사용.
		for (size_t j = 0; j <= dep; j++)
		{
			const size_t level = dep - j;
			const EncryptionParameters& params = context_.param(level);

			if (j == 0)
			{
				create_secret_key_internal(params, sk);
			}
			else
			{
				sk.reset(sk.row_size(), sk.col_size(), d - 1, params.q());
			}

			sk_.data(level, sk);
		}
	}

	SecretKey KeyGenerator::secret_key() const
	{
		return sk_;
	}

	void KeyGenerator::create_public_key(PublicKey& destination) const
	{
		const uint64_t dep = context_.depth();
		const uint64_t d = context_.poly_modulus_degree();

		PolyMatrix pk;
		destination.assign(dep + 1);

		// 하나의 공개키 생성.
		for (size_t j = 0; j <= dep; j++)
		{	
			const size_t level = dep - j;
			const EncryptionParameters& params = context_.param(level);

			if (j == 0)
			{
				create_public_key_internal(sk_.data(level, 2), params, 1, pk);
			}
			else
			{
				pk.reset(pk.row_size(), pk.col_size(), d - 1, params.q());
			}

			destination.data(level, pk);
		}
	}

	void KeyGenerator::create_relin_keys(RelinKeys& destination) const
	{
		const uint64_t dep = context_.depth();
		const uint64_t d = context_.poly_modulus_degree();

		PolyMatrix rk;
		destination.assign(dep + 1);

		// 하나의 재선형화 키 생성.
		for (size_t j = 0; j <= dep; j++)
		{
			const size_t level = dep - j;
			const EncryptionParameters& params = context_.param(level);

			if (j == 0)
			{
				create_relin_key_internal(sk_.data(level, 3), sk_.data(level, 2), context_, params, rk);
			}
			else
			{
				rk.reset(rk.row_size(), rk.col_size(), d - 1, params.q());
			}

			destination.data(level, rk);
		}
	}

	void KeyGenerator::create_relin_key_internal(const PolyMatrix& secret_key1, const PolyMatrix& secret_key2, const Context& context, const EncryptionParameters& params, PolyMatrix& destination) const
	{
		const uint64_t d = context.poly_modulus_degree();
		const uint64_t q = params.q();
		const uint64_t log_q = static_cast<uint64_t>(std::ceill(std::log2l(params.q())));
		const uint64_t N = secret_key1.row_size() * log_q;

		// A: 공개키 데이터. (Nx2 poly matrix)
		PolyMatrix A;
		create_public_key_internal(secret_key2, params, N, A);

		// W: 비밀키1의 power of 2. (Nx1 poly matrix)
		PolyMatrix W;
		W.assign(N, 1, d - 1, q);

		for (size_t r = 0; r < secret_key1.row_size(); r++)
		{
			for (size_t c = 0, pow = 1; c < log_q; c++, pow <<= 1)
			{
				W.set((r * log_q) + c, 0, secret_key1.get(r, 0) * pow);
			}
		}

		// B: 재선형화 키. (Nx2 poly matrix)
		destination = A;
		for (size_t r = 0; r < N; r++)
		{
			destination.set(r, 0, destination.get(r, 0) + W.get(r, 0));
		}
	}

	void KeyGenerator::create_secret_key_internal(const EncryptionParameters& params, PolyMatrix& destination) const
	{
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t q = params.q();

		// sk: 비밀키 데이터. (2x1 poly matrix)
		destination.assign(2, 1, d - 1, q);

		// sk_0: 상수 다항식. (d-1 polynomial)
		// sk_0 = 1 + 0*x + ... + 0*x^(d-1)
		destination.set(0, 0, 0, static_cast<uint64_t>(1));

		// sk_1: 가우시안 분포에서 뽑은 다항식. (d-1 polynomial)
		// sk_1 = X_0 + X_1*x + ... + X_(d-1)*x^(d-1)
		destination.set(1, 0, sample_poly_from_gaussian_dist(context_, params));
	}

	
	void KeyGenerator::create_public_key_internal(const PolyMatrix& secret_key, const EncryptionParameters& params, const uint64_t N, PolyMatrix& destination) const
	{
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = params.q();

		// pk: 공개키 데이터. (Nx2 poly matrix)
		destination.assign(N, 2, d - 1, q);

		// t: sk[1][0]로 설정. (1x1 poly matrix)
		PolyMatrix t(1, 1, d - 1, q);
		t.set(0, 0, secret_key.get(1, 0));

		// B: Zq 균등 분포에서 뽑은 다항식 행렬. (Nx1 poly matrix)
		// B[r][c] = Z_0 + Z_1*x + ... + Z_(d-1)*x^(d-1)
		PolyMatrix B(N, 1, d - 1, q);
		for (uint64_t r = 0; r < N; r++)
		{
			B.set(r, 0, sample_poly_from_uniform_dist(context_, params));
		}

		// e: 가우시안 분포에서 뽑은 다항식 행렬. (Nx1 poly matrix)
		// e[r][c] = X_0 + X_1*x + ... + X_(d-1)*x^(d-1)
		PolyMatrix e(N, 1, d - 1, q);
		for (uint64_t r = 0; r < N; r++)
		{
			e.set(r, 0, sample_poly_from_gaussian_dist(context_, params));
		}

		// b = Bt + pe. (Nx1 poly matrix)
		PolyMatrix b = (B * t) + (e * p);

		// pk = (b, -B). (Nx2 poly matrix)
		for (uint64_t r = 0; r < N; r++)
		{
			destination.set(r, 0, b.get(r, 0));
			destination.set(r, 1, -(B.get(r, 0)));
		}
	}
}