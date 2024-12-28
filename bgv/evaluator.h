#pragma once

#include "context.h"
#include "publickey.h"
#include "ciphertext.h"

namespace fheprac
{
	class Evaluator
	{
	public:
		Evaluator(Context& context);

		void mod_switch(Ciphertext& ciphertext, Ciphertext& destination);

	private:
		uint64_t drop_to_next_q(uint64_t value, uint64_t curr_q, uint64_t next_q, uint64_t p) const;

		Context context_;
	};
}