#pragma once

#include "context.h"
#include "secretkey.h"
#include "plaintext.h"
#include "ciphertext.h"

namespace fheprac
{
	class Decryptor
	{
	public:
		Decryptor(Context& context, SecretKey& secretkey);

		void decrypt(Ciphertext& ciphertext, Plaintext& destination);

	private:
		Context context_;

		SecretKey sk_;
	};
}