#include "decryptor.h"
#include <vector>

namespace fheprac
{
	Decryptor::Decryptor(Context& context, SecretKey& secretkey) :context_(context), sk_(secretkey) {}

	void Decryptor::decrypt(Ciphertext& ciphertext, Plaintext& destination)
	{
		EncryptionParameters params = ciphertext.param();

		const uint64_t l = params.l();
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = params.q();

		// TODO: 예외처리
		// 암호문의 레벨에 해당하는 sk가 없는 경우

		// ct: 암호문 데이터 (2x1 poly matrix)
		PolyMatrix ct = ciphertext.data();

		// sk: 레벨dep에서 정의된 비밀키 (2x1 poly matrix)
		// sk: [[1], [t]]
		PolyMatrix sk = sk_.data(l);

		// m_q: 복호화 데이터 (1x1 poly matrix)
		// m_q = [<c^T, s>]_q
		PolyMatrix mq = ct.t() * sk;

		// m: 평문 데이터 (1x1 poly matrix)
		// m = [m_q]_p
		destination.data().assign(1, 1, d - 1, p);
		for (uint64_t i = 0; i < d; i++)
		{
			destination.data().set(0, 0, i, mq.get(0, 0, i));
		}
	}
}