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

		// �����ڿ��� ���Ű�� ����
		std::vector<PolyMatrix> sk(dep + 1);

		for (uint64_t j = 0; j <= dep; j++)
		{
			EncryptionParameters params = context_.param(j);
			const uint64_t q = params.q();

			// d-1�� ���׽��� ���ҷ� ���� 2 x 1 ���.
			sk[j].assign(2, 1, d - 1, q);

			// sk�� ù ��° ����: ��� ���׽����� ����.
			// sk[0] = 1 + 0*x + ... + 0*x^(d-1)
			sk[j].set(0, 0, 0, static_cast<uint64_t>(1));

			// sk�� �� ��° ����: ����þ� �������� ���� ��� ���׽����� ����,
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

			// d-1�� ���׽��� ���ҷ� ���� 2 x 1 ���.
			pk[j].assign(1, 2, d - 1, q);

			// t: sk[1][0]�� ����.
			Polynomial t = sk_.data(j).get(1, 0);

			// B: Zq �յ� �������� ���� ���׽����� ����,
			// B = z_0 + z_1*x + ... + z_(d-1)*x^(d-1)
			Polynomial B = sample_poly_from_uniform_dist(context_, params);  // sample_poly_from_uniform_dist

			// e: ����þ� �������� ���� ���׽����� ����,
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
		// TODO: Key Switching ���� ���� �ʿ�.
	}

	
	void KeyGenerator::create_public_key_internal(SecretKey& secret_key, PublicKey& destination)
	{
		// TODO: Key Switching�� public key N * n ũ���� ������ �ʿ���. 
		// ���� ����Ű�� �����ϴ� ���� ������ �����ѵ� ����Ű, �缱��ȭŰ ���� �Լ����� ȣ���ؼ� ������ ������ �ʿ䰡 ����.
	}
}