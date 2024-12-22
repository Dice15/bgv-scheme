#include "keygenerator.h"
#include "util/modular.h"
#include <Eigen/Dense>
#include <vector>
#include <iostream>

namespace fheprac
{
	KeyGenerator::KeyGenerator(Context& context) : context_(context)
	{
		const uint64_t l = context_.level();
		const uint64_t d = context_.poly_modulus_degree();

		// 생성자에서 비밀키를 생성
		std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>> sk(
			l + 1, std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>(2, Eigen::Vector<int64_t, Eigen::Dynamic>(d)));

		for (uint64_t j = 0; j <= context_.level(); j++)
		{
			EncryptionParameters param = context_.param(j);

			// sk의 두 번째 요소(d-1차 다항식)는 1을 계수로 설정한다.
			for (uint64_t i = 0; i < d; i++)
			{
				sk[j][0][i] = static_cast<int64_t>(1);
			}
			
			// sk의 두 번째 요소(d-1차 다항식)는 x(가우시안 분포)에서 뽑은 d개의 값을 계수로 설정한다.
			for (uint64_t i = 0; i < d; i++)
			{
				sk[j][1][i] = static_cast<int64_t>(context_.value_from_gaussian_dist());
			}
		}

		secret_key_ = SecretKey(sk);
	}

	SecretKey KeyGenerator::get_secret_key() const
	{
		return secret_key_;
	}

	void KeyGenerator::create_public_key(SecretKey &secret_key, PublicKey& destination)
	{
		const uint64_t l = context_.level();
		const uint64_t d = context_.poly_modulus_degree();
		const int64_t p = static_cast<int64_t>(context_.plain_modulus_value());

		std::vector<std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>> pk(
			l + 1, std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>>(2, Eigen::Vector<int64_t, Eigen::Dynamic>(d)));

		for (uint64_t j = 0; j <= context_.level(); j++)
		{
			EncryptionParameters param = context_.param(j);
			const int64_t q = static_cast<int64_t>(param.q());
			const int64_t h_q = q / static_cast<int64_t>(2);

			Eigen::Vector<int64_t, Eigen::Dynamic> t = secret_key.key(j)[1];
			Eigen::Vector<int64_t, Eigen::Dynamic> B(d);
			Eigen::Vector<int64_t, Eigen::Dynamic> e(d);

			for (uint64_t i = 0; i < d; i++)
			{
				B[i] = static_cast<int64_t>(context_.value_from_gaussian_dist()); //static_cast<int64_t>(param.value_from_uniform_dist());
				e[i] = static_cast<int64_t>(context_.value_from_gaussian_dist());
			}

			Eigen::Vector<int64_t, Eigen::Dynamic> b;
			
			b = B.cwiseProduct(t); 
			mod(b, q);

			b += (p * e);
			mod(b, q);

			pk[j][0] = b;
			pk[j][1] = -B;
			//negate(pk[j][1], q);

			/*std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> A = pk[j];
			std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> s = secret_key.key(j);

			Eigen::Vector<int64_t, Eigen::Dynamic> temp1 = A[0].cwiseProduct(s[0]);
			mod(temp1, q);

			Eigen::Vector<int64_t, Eigen::Dynamic> temp2 = A[1].cwiseProduct(s[1]);
			mod(temp2, q);

			Eigen::Vector<int64_t, Eigen::Dynamic> temp3 = B + pk[j][1];//temp1 + temp2;
			mod(temp3, q);

			std::cout << "\ntest1\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << A[0][i] << ' ' << s[0][i] << ' ' << temp1[i] << '\n';
			}

			std::cout << "\ntest2\n";
			auto temp4 = B.cwiseProduct(t);
			for (int i = 0; i < d; i++)
			{
				std::cout << A[1][i] << ' ' << s[1][i] << ' ' << temp2[i] << ' ' << temp4[i] << '\n';
			}

			std::cout << "\ntest3\n";
			for (int i = 0; i < d; i++)
			{
				std::cout << p * e[i] << ' ' << temp3[i] << ' ' << temp3[i] % p << '\n';
			}*/
		}

		destination = PublicKey(pk);
	}
}