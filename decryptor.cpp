#include "decryptor.h"
#include "util/modular.h"
#include <Eigen/Dense>
#include <vector>

namespace fheprac
{
	Decryptor::Decryptor(Context& context, SecretKey& secretkey) :context_(context), sk_(secretkey) {}

	void Decryptor::decrypt(Ciphertext& ciphertext, Plaintext& destination)
	{
		const EncryptionParameters param = ciphertext.param();

		const uint64_t l = param.l();
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = param.q();

		destination.assign(d);

		const std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> sk = sk_.key(l);

		for (uint64_t i = 0; i < d; i++)
		{
			destination[i] = ((ciphertext(0, i) * sk[0][i]) + (ciphertext(1, i) * sk[1][i])) % q % p;
		}
	}
}