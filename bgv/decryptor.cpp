#include "decryptor.h"
#include <vector>

namespace fheprac
{
	Decryptor::Decryptor(Context& context, SecretKey& secretkey) :context_(context), sk_(secretkey) {}

	void Decryptor::decrypt(Ciphertext& ciphertext, Plaintext& destination)
	{
		EncryptionParameters params = ciphertext.param();

		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t q = params.q();

		// TODO: ����ó��
		// ��ȣ���� ������ �ش��ϴ� sk�� ���� ���

		// ct: ��ȣ�� ������. (2x1 poly matrix)
		PolyMatrix ct = ciphertext.data();

		// sk: ����dep���� ���ǵ� ���Ű. (2x1 poly matrix)
		// sk: [[1], [t]]
		PolyMatrix sk = sk_.data(context_.depth());

		if (ct.modulus() < sk.modulus())
		{
			sk.reset(2, 1, d - 1, ct.modulus());
		}

		// m_q: ��ȣȭ ������. (1x1 poly matrix)
		// m_q = [<c^T, s>]_q
		destination.data() = ct.t() * sk;

		// m: �� ������. (1x1 poly matrix)
		// m = [m_q]_p
		destination.data().reset(1, 1, d - 1, p);
	}
}