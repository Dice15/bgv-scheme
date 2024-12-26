#include "encryptor.h"
#include "util/distribution.h"
#include "util/polynomial.h"
#include "util/polymatrix.h"
#include <vector>
#include <iostream>

namespace fheprac
{
	Encryptor::Encryptor(Context& context, PublicKey& publickey) :context_(context), pk_(publickey) {}

	void Encryptor::encrypt(Plaintext& plaintext, Ciphertext& destination)
	{
		EncryptionParameters params = context_.first_param();

		const uint64_t dep = context_.depth();
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = params.q();

		// pt: 평문 데이터 (1x1 poly matrix)
		PolyMatrix pt = plaintext.data();

		// m: 메시지 (2x1 poly matrix)
		// m = (pt_0 + pt_1*x + ... + pt_(d-1)*x) mod q
		PolyMatrix m(2, 1, d - 1, q);
		for (uint64_t i = 0; i < d; i++)
		{
			m.set(0, 0, i, pt.get(0, 0, i));
			m.set(1, 0, i, static_cast<uint64_t>(0));
		}

		// e: 가우시안 분포에서 뽑은 다항식 행렬 (2x1 poly matrix)
		// e[0][0] = X_0_0 + X_0_1*x + ... + X_0_(d-1)*x^(d-1)
		// e[1][0] = X_1_0 + X_1_1*x + ... + X_1_(d-1)*x^(d-1)
		PolyMatrix e(2, 1, d - 1, q);
		e.set(0, 0, sample_poly_from_gaussian_dist(context_, params));
		e.set(1, 0, sample_poly_from_gaussian_dist(context_, params));

		// r: 가우시안 분포에서 뽑은 상수 다항식 행렬 (1x1 poly matrix)
		// r = X + 0*x + ... + 0*x^(d-1)
		PolyMatrix r(1, 1, d - 1, q);
		r.set(0, 0, 0, sample_from_gaussian_dist(context_, params));

		// pk: 레벨dep에서 정의된 공개키 (1x2 poly matrix)
		// pk: [[b, -B]]
		PolyMatrix pk = pk_.data(dep);

		// ct: 암호문 데이터 (2x1 poly matrix)
		// ct = m + pe + (pk^T)r
		destination.data() = m + (e * p) + (pk.t() * r);
		destination.param() = params;
	}
}