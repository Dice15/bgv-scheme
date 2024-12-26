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

		SecretKey get_secret_key() const;

		void create_public_key(PublicKey& destination);

		void create_relin_keys(RelinKeys& destination);

	private:
		void create_public_key_internal(SecretKey& secret_key, PublicKey& destination);

		SecretKey sk_;

		Context context_;
	};
}