#include "encoder.h"
#include "util/polymatrix.h"
#include <stdexcept>

namespace fheprac
{
	Encoder::Encoder(Context& context) :context_(context) {}

	void Encoder::encode(std::vector<int64_t>& message, Plaintext& destination) const
	{
		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();

		// pt: ∆ÚπÆ. (1x1 poly matrix)
		// pt = (v_0 + v_1*x + ... + v_(d-1)*x) mod p
		destination.data().assign(1, 1, d - 1, p);

		for (uint64_t i = 0; i < d; i++)
		{
			if (std::abs(message[i]) > p / 2)
			{
				throw std::out_of_range("Value out of range.");
			}

			if (message[i] < 0)
			{
				destination.data().set(0, 0, i, static_cast<uint64_t>(static_cast<int64_t>(p) - std::abs(message[i])));
			}
			else
			{
				destination.data().set(0, 0, i, message[i]);
			}
		}
	}

	void Encoder::decode(Plaintext& plaintext, std::vector<int64_t>& destination) const
	{
		const uint64_t slot_count = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t h_p = p / static_cast<uint64_t>(2);

		// v: ∫§≈Õ. (d-vetor)
		// v = (pt_0 + pt_1*x + ... + pt_(d-1)*x) mod p
		destination.assign(slot_count, 0);

		for (uint64_t i = 0; i < slot_count; i++)
		{
			if (plaintext.data().get(0, 0, i) > h_p)
			{
				destination[i] = static_cast<int64_t>(plaintext.data().get(0, 0, i)) - static_cast<int64_t>(p);
			}
			else
			{
				destination[i] = plaintext.data().get(0, 0, i);
			}
		}
	}
}