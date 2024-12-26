#include "evaluator.h"
#include "encryptionparams.h"
#include <stdexcept>
#include <iostream>

namespace fheprac
{
	Evaluator::Evaluator(Context& context) :context_(context) {}

	void Evaluator::mod_switch(Ciphertext& ciphertext, Ciphertext& destination)
	{
		EncryptionParameters curr_params = ciphertext.param();
		EncryptionParameters next_params = context_.param(curr_params.next_param_index());

		if (curr_params.l() <= next_params.l())
		{
			throw std::out_of_range("Modulus switching cannot proceed further.");
		}

		const uint64_t d = context_.poly_modulus_degree();
		const uint64_t p = context_.plain_modulus_value();
		const uint64_t curr_q = curr_params.q();
		const uint64_t next_q = next_params.q();

		// ct, ct': 암호문 데이터 (2x1 poly matrix)
		PolyMatrix ct = ciphertext.data();
		PolyMatrix ct_dot(2, 1, d - 1, next_q);

		// curr_q > next_q > p and curr_q = next_q = 1 mod p
		// [<ct', s>]_next_q = [<ct, s>]_curr_q mod p 
		// ct' = closest((curr_q / next_q) * ct) mod p
		for (uint64_t r = 0; r < ct_dot.row_size(); r++)
		{
			for (uint64_t c = 0; c < ct_dot.col_size(); c++)
			{
				for (uint64_t i = 0; i <= ct_dot.degree(); i++)
				{
					double_t coeff = static_cast<double_t>(ct.get(r, c, i));
					double_t ratio = static_cast<double_t>(next_q) / static_cast<double_t>(curr_q);
					double_t scaled = ratio * coeff;
					int64_t rounded = std::floorl(scaled + 0.5);

					if (rounded < 0)
					{
						ct_dot.set(r, c, i, static_cast<uint64_t>(next_q - (std::abs(rounded) % next_q)));
					}
					else
					{
						ct_dot.set(r, c, i, static_cast<uint64_t>(rounded));
					}
				}
			}
		}

		destination.data() = ct_dot;
		destination.param() = next_params;
	}
}


/*double_t coeff = static_cast<double_t>(ct.get(r, c, i));
double_t scale = static_cast<double_t>(next_q) / static_cast<double_t>(curr_q);
double_t diff = (scale * coeff) - coeff;
double_t ratio = diff / static_cast<double_t>(p);
int64_t k = static_cast<int64_t>(std::llround(ratio)); // Zq위이므로 uint64_t로 바꿔야함

ct_dot.set(r, c, i, ct.get(r, c, i) + (k * p));*/
/*
			for (uint64_t i = 0; i < context_.poly_modulus_degree(); i++)
			{
				double_t c = static_cast<double_t>(ciphertext(r, i));
				double_t scale = static_cast<double_t>(next_q) / static_cast<double_t>(curr_q);
				double_t diff = (scale * c) - c;
				double_t ratio = diff / static_cast<double_t>(p);
				int64_t k = static_cast<int64_t>(std::llround(ratio));

				result(r, i) = ciphertext(r, i) + (k * p);
			}

std::cout << "\n";
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