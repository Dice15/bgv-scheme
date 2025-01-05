#pragma once

#include "context.h"
#include "relinkeys.h"
#include "ciphertext.h"

namespace fheprac
{
	class Evaluator
	{
	public:
		Evaluator(const Context& context);

		void mod_switch(const Ciphertext& ciphertext, Ciphertext& destination) const;

		void relinearize(const Ciphertext& ciphertext, RelinKeys& relinKeys, Ciphertext& destination) const;

		void add(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const;

		void sub(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const;

		void multiply(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const;

	private:
		uint64_t drop_to_next_q(uint64_t value, const uint64_t curr_q, const uint64_t next_q, const uint64_t p) const;

		Context context_;
	};
}