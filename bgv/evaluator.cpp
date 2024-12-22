#include "evaluator.h"
#include "encryptionparams.h"
#include <iostream>

namespace fheprac
{
	Evaluator::Evaluator(Context& context) :context_(context) {}

	void Evaluator::mod_switch(Ciphertext& ciphertext, Ciphertext& destination)
	{
		EncryptionParameters curr_param = ciphertext.param();
		EncryptionParameters next_param = context_.param(curr_param.next_param_index());

		if (curr_param.l() <= next_param.l())
		{
			// TODO: 더이상 모듈러스 스위칭을 할 수 없으므로 에러처리
			return;
		}

		const uint64_t d = context_.poly_modulus_degree();
		const int64_t p = static_cast<int64_t>(context_.plain_modulus_value());
		const int64_t curr_q = static_cast<int64_t>(curr_param.q());
		const int64_t next_q = static_cast<int64_t>(next_param.q());

		Ciphertext result;
		result.assign(ciphertext.poly_count(), ciphertext.slot_count());
		result.param() = next_param;
		result.param().l() = curr_param.l();

		for (uint64_t t = 0; t < 2; t++)
		{
			for (uint64_t i = 0; i < context_.poly_modulus_degree(); i++)
			{
				double_t c = static_cast<double_t>(ciphertext(t, i));
				double_t scale = static_cast<double_t>(next_q) / static_cast<double_t>(curr_q);
				double_t diff = (scale * c) - c;
				double_t ratio = diff / static_cast<double_t>(p);
				int64_t k = static_cast<int64_t>(std::llround(ratio));

				result(t, i) = ciphertext(t, i) + (k * p);

				/*std::cout << "\n";
				std::cout << "t: " << p << '\n';
				std::cout << "curr_q: " << curr_q << '\n';
				std::cout << "next_q: " << next_q << '\n';
				std::cout << "c: " << ciphertext(t, i) << '\n';
				std::cout << "scale: " << scale << '\n';
				std::cout << "diff: " << diff << '\n';
				std::cout << "ratio: " << ratio << '\n';
				std::cout << "k: " << k << '\n';
				std::cout << "c': " << destination(t, i) << '\n';
				std::cout << "c mod t: " << ciphertext(t, i) % p << '\n';
				std::cout << "c' mod t: " << destination(t, i) % p << '\n';*/
			}
		}

		destination = result;
	}
}