#include "encoder.h"

namespace fheprac
{
	Encoder::Encoder(Context& context) :context_(context) {}

	void Encoder::encode(std::vector<int64_t>& message, Plaintext& destination) const
	{
		// TODO: 절대값이 p/2 초과인 수가 있다면 에러처리
		const uint64_t slot_count = context_.poly_modulus_degree();
		const int64_t p = static_cast<int64_t>(context_.plain_modulus_value());

		destination.assign(slot_count);

		for (uint64_t i = 0; i < slot_count; i++)
		{
			if (message[i] < 0)
			{
				destination[i] = message[i] + p;
			}
			else
			{
				destination[i] = message[i];
			}		
		}
	}

	void Encoder::decode(Plaintext& plaintext, std::vector<int64_t>& destination) const
	{
		const uint64_t slot_count = context_.poly_modulus_degree();
		const int64_t p = static_cast<int64_t>(context_.plain_modulus_value());
		const int64_t h_p = p / static_cast<int64_t>(2);

		destination.assign(slot_count, 0);

		for (uint64_t i = 0; i < slot_count; i++)
		{
			if (plaintext[i] > h_p)
			{
				destination[i] = plaintext[i] - p;
			}
			else
			{
				destination[i] = plaintext[i];
			}
		}
	}
}