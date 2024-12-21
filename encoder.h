#pragma once

#include "context.h"
#include "plaintext.h"

namespace fheprac
{
	class Encoder
	{
	public:
		Encoder(Context& context);

		void encode(std::vector<int64_t>& message, Plaintext& destination) const;

		void decode(Plaintext& plaintext, std::vector<int64_t>& destination) const;

	private:
		Context context_;
	};
}