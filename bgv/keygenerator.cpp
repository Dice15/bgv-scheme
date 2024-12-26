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
		const uint64_t d = context_.poly_modulus_degree();

		// 생성자에서 비밀키를 생성
		std::vector<PolyMatrix> sk(dep + 1);

		for (uint64_t j = 0; j <= dep; j++)
		{
			EncryptionParameters params = context_.param(j);
			const uint64_t q = params.q();

			// d-1차 다항식을 원소로 갖는 2 x 1 행렬.
			sk[j].assign(2, 1, d - 1, q);

			// sk의 첫 번째 원소: 상수 다항식으로 설정.
			// sk[0] = 1 + 0*x + ... + 0*x^(d-1)
			sk[j].set(0, 0, 0, static_cast<uint64_t>(1));

			// sk의 두 번째 원소: 가우시안 분포에서 뽑은 상수 다항식으로 설정,
			// sk[1] = t + 0*x + ... + 0*x^(d-1)
			sk[j].set(1, 0, 0, sample_from_gaussian_dist(context_, params));
		}

		sk_ = SecretKey(sk);
	}

	SecretKey KeyGenerator::get_secret_key() const
	{
		return sk_;
	}

	void KeyGenerator::create_public_key(PublicKey& destination)
	{
		const uint64_t dep = context_.depth();
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();

		std::vector<PolyMatrix> pk(dep + 1);

		for (uint64_t j = 0; j <= dep; j++)
		{
			EncryptionParameters params = context_.param(j);
			const uint64_t q = params.q();

			// d-1차 다항식을 원소로 갖는 2 x 1 행렬.
			pk[j].assign(1, 2, d - 1, q);

			// t: sk[1][0]로 설정.
			Polynomial t = sk_.data(j).get(1, 0);

			// B: Zq 균등 분포에서 뽑은 다항식으로 설정,
			// B = z_0 + z_1*x + ... + z_(d-1)*x^(d-1)
			Polynomial B = sample_poly_from_uniform_dist(context_, params);  // sample_poly_from_uniform_dist

			// e: 가우시안 분포에서 뽑은 다항식으로 설정,
			// e = e_0 + e_1*x + ... + e_(d-1)^(d-1)
			Polynomial e = sample_poly_from_gaussian_dist(context_, params);

			// b = Bt + pe
			Polynomial b = (B * t) + (e * p);

			// pk = (b, -B)
			pk[j].set(0, 0, b);
			pk[j].set(0, 1, -B);


			// TEST Code
			PolyMatrix text_A = pk[j];
			PolyMatrix text_s = sk_.data(j);
			PolyMatrix text_e = text_A * text_s;

			std::cout << "\n===========================\n";
			std::cout << "\ntest: A*s = p*e\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << e.get(i) << " -> " << (p * e.get(i)) % q << " = " << text_e.get(0, 0, i) << " -> " << text_e.get(0, 0, i) % p << '\n';
			}

			/*std::cout << "\ntest: pk\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << b.get(i) << ", " << B.get(i) << ", " << (-B).get(i) << '\n';
			}

			std::cout << "\ntest: t\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << t.get(i) << '\n';
			}

			std::cout << "\ntest: B*t vs pk*sk\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << (B * t).get(i) << ' ' << text_e.get(0, 0, i) << '\n';
			}*/
		}

		destination = PublicKey(pk);
	}

	void KeyGenerator::create_relin_keys(RelinKeys& destination)
	{
		// TODO: Key Switching 로직 구현 필요.
	}

	
	void KeyGenerator::create_public_key_internal(SecretKey& secret_key, PublicKey& destination)
	{
		// TODO: Key Switching시 public key N * n 크기의 생성이 필요함. 
		// 따라서 공개키를 생성하는 공통 로직을 구현한뒤 공개키, 재선형화키 생성 함수에서 호출해서 쓰도록 수정할 필요가 있음.
	}
}