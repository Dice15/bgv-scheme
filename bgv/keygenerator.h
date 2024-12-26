#pragma once

#include "context.h"
#include "secretkey.h"
#include "publickey.h"
#include "relinkeys.h"

namespace fheprac
{
	class KeyGenerator
	{
	public:
		KeyGenerator(Context& context);

		SecretKey secret_key() const;

		void create_public_key(PublicKey& destination);

		void create_relin_keys(RelinKeys& destination);

	private:
		void create_secret_key_internal(const EncryptionParameters& params, PolyMatrix& destination) const;

		void create_public_key_internal(const PolyMatrix& secret_key, const EncryptionParameters& params, const uint64_t N, PolyMatrix& destination) const;

		SecretKey sk_;

		Context context_;
	};
}