#pragma once

#include "context.h"
#include "publickey.h"
#include "plaintext.h"
#include "ciphertext.h"

namespace fheprac
{
	class Encryptor
	{
	public:
		Encryptor(Context& context, PublicKey &publickey);

		void encrypt(Plaintext& plaintext, Ciphertext& destination);

	private:
		Context context_;

		PublicKey pk_;
	};
}