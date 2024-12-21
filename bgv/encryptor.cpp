#include "encryptor.h"
#include "util/modular.h"
#include <Eigen/Dense>
#include <vector>

namespace fheprac
{
	Encryptor::Encryptor(Context& context, PublicKey& publickey) :context_(context), pk_(publickey) {}

	void Encryptor::encrypt(Plaintext& plaintext, Ciphertext& destination)
	{
		const EncryptionParameters param = context_.first_param();

		const uint64_t l = context_.level();
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = param.q();


		destination.assign(2, d);
		destination.param() = param;

		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> m(2, Eigen::Vector<int64_t, Eigen::Dynamic>(d));
		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> e(2, Eigen::Vector<int64_t, Eigen::Dynamic>(d));
		Eigen::Vector<int64_t, Eigen::Dynamic> r(d);

		for (uint64_t i = 0; i < d; i++)
		{
			m[0][i] = plaintext[i] % q;
			m[1][i] = static_cast<int64_t>(0);

			e[0][i] = (static_cast<int64_t>(p) * static_cast<int64_t>(context_.value_from_gaussian_dist())) % static_cast<int64_t>(q);
			e[1][i] = (static_cast<int64_t>(p) * static_cast<int64_t>(context_.value_from_gaussian_dist())) % static_cast<int64_t>(q);

			r[i] = static_cast<int64_t>(context_.value_from_gaussian_dist()) % q;
		}

		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> c = pk_.key(l);

		for (uint64_t t = 0; t < 2; t++)
		{
			c[t] *= r;
			mod(c[t], static_cast<int64_t>(q));

			c[t] += e[t];
			mod(c[t], static_cast<int64_t>(q));

			c[t] += m[t];
			mod(c[t], static_cast<int64_t>(q));

			for (uint64_t i = 0; i < d; i++)
			{
				destination(t, i) = c[t][i];
			}
		}
	}
}