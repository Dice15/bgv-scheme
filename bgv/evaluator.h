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
		Context context_;
	};
}