#include "keygenerator.h"
#include "util/distribution.h"
#include "util/polynomial.h"
#include "util/polymatrix.h"
#include <vector>
#include <iostream>

namespace fheprac
{
	KeyGenerator::KeyGenerator(Context& context) : context_(context)
	{
		const uint64_t dep = context_.depth();

		std::vector<PolyMatrix> sk(dep + 1);

		// depth 만큼 비밀키 생성
		for (uint64_t j = 0; j <= dep; j++)
		{
			create_secret_key_internal(context_.param(j), sk[j]);
		}

		sk_ = SecretKey(sk);
	}

	SecretKey KeyGenerator::secret_key() const
	{
		return sk_;
	}

	void KeyGenerator::create_public_key(PublicKey& destination)
	{
		const uint64_t dep = context_.depth();

		std::vector<PolyMatrix> pk(dep + 1);

		// depth 만큼 공개키 생성
		for (uint64_t j = 0; j <= dep; j++)
		{	
			create_public_key_internal(sk_.data(j), context_.param(j), 1, pk[j]);
		}

		destination = PublicKey(pk);
	}

	void KeyGenerator::create_relin_keys(RelinKeys& destination)
	{
		// TODO: Key Switching 로직 구현 필요.
		// 
		// uint64_t N = static_cast<uint64_t>(std::ceil(std::log2(params.q())));
	}

	void KeyGenerator::create_secret_key_internal(const EncryptionParameters& params, PolyMatrix& destination) const
	{
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t q = params.q();

		// sk: 비밀키 데이터 (2x1 poly matrix).
		destination.assign(2, 1, d - 1, q);

		// sk[0]: 상수 다항식 (d-1 polynomial).
		// sk[0] = 1 + 0*x + ... + 0*x^(d-1)
		destination.set(0, 0, 0, static_cast<uint64_t>(1));

		// sk[1]: 가우시안 분포에서 뽑은 상수 다항식 (d-1 polynomial).
		// sk[1] = t + 0*x + ... + 0*x^(d-1)
		destination.set(1, 0, 0, sample_from_gaussian_dist(context_, params));
	}

	
	void KeyGenerator::create_public_key_internal(const PolyMatrix& secret_key, const EncryptionParameters& params, const uint64_t N, PolyMatrix& destination) const
	{
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = params.q();

		// pk: 공개키 데이터 (Nx2 poly matrix).
		destination.assign(N, 2, d - 1, q);

		// t: sk[1][0]로 설정 (1x1 poly matrix).
		PolyMatrix t(1, 1, d - 1, q);
		t.set(0, 0, secret_key.get(1, 0));

		// B: Zq 균등 분포에서 뽑은 다항식 행렬 (Nx1 poly matrix).
		// B[r][c] = z_0 + z_1*x + ... + z_(d-1)*x^(d-1)
		PolyMatrix B(N, 1, d - 1, q);
		for (uint64_t r = 0; r < N; r++)
		{
			B.set(r, 0, sample_poly_from_uniform_dist(context_, params));
		}

		// e: 가우시안 분포에서 뽑은 다항식 행렬 (Nx1 poly matrix).
		// e[r][c] = e_0 + e_1*x + ... + e_(d-1)^(d-1)
		PolyMatrix e(N, 1, d - 1, q);
		for (uint64_t r = 0; r < N; r++)
		{
			e.set(r, 0, sample_poly_from_gaussian_dist(context_, params));
		}

		// b = Bt + pe (Nx1 poly matrix).
		PolyMatrix b = (B * t) + (e * p);

		// pk = (b, -B)
		for (uint64_t r = 0; r < N; r++)
		{
			destination.set(r, 0, b.get(r, 0));
			destination.set(r, 1, -(B.get(r, 0)));
		}

		// TEST Code
		PolyMatrix text_A = destination;
		PolyMatrix text_s = secret_key;
		PolyMatrix text_e = text_A * text_s;

		std::cout << "\n===========================\n";
		std::cout << "\ntest: A*s = p*e\n";
		for (int r = 0; r < N; r++)
		{
			for (int i = 0; i < d; i++)
			{
				std::cout << e.get(r, 0).get(i) << " -> " << (p * e.get(r, 0).get(i)) % q << " = " << text_e.get(0, 0, i) << " -> " << text_e.get(0, 0, i) % p << '\n';
			}
		}
	}
}