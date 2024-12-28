#include "evaluator.h"
#include "encryptionparams.h"
#include <stdexcept>
#include <iostream>
#include <intrin.h>

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


        // ct, ct': 암호문 데이터. (2x1 poly matrix)
        PolyMatrix ct = ciphertext.data();
        PolyMatrix ct_dot(2, 1, d - 1, next_q);

        // curr_q > next_q > p and curr_q = next_q = 1 mod p.
        // [<ct', s>]_next_q = [<ct, s>]_curr_q mod p.
        // ct' = closest((curr_q / next_q) * ct) mod p.
        for (uint64_t row = 0; row < ct_dot.row_size(); row++)
        {
            for (uint64_t col = 0; col < ct_dot.col_size(); col++)
            {
                for (uint64_t i = 0; i <= ct_dot.degree(); i++)
                {
                    uint64_t coeff_curr_q = ct.get(row, col, i);
                    uint64_t coeff_next_q = drop_to_next_q(coeff_curr_q, curr_q, next_q, p);
                    ct_dot.set(row, col, i, coeff_next_q);
                }
            }
        }

        destination.data() = ct;
        destination.param() = next_params;
    }

    uint64_t Evaluator::drop_to_next_q(uint64_t value, uint64_t curr_q, uint64_t next_q, uint64_t p) const
    {
        const uint64_t curr_q_h = curr_q >> 1;
        const uint64_t p_h = p >> 1;
   
        /*
        * 계산 정확도를 위해 소수점을 사용하지 않고 계산.
        * Origin code. (using point)
        uint64_t scaled = ((value * next_q) + curr_q_h) / curr_q;
        uint64_t diff = scaled - value;
        uint64_t ratio = static_cast<uint64_t>(std::llround(static_cast<double_t>(diff) / static_cast<double_t>(p)));
        uint64_t result = value + (ratio * p);
        */

        // scaled = {(value * next_q) + (curr_q / 2)} / curr_q. (= (next_q / curr_q) * value + 0.5)
        uint64_t high, low, carry, remainder, scaled;
        low = _umul128(value, next_q, &high);
        carry = (low + curr_q_h < low);
        low += curr_q_h;
        high += carry;
        scaled = _udiv128(high, low, curr_q, &remainder);

        // diff = scaled - value.
        uint64_t diff = scaled - value;

        // ratio = (diff + p_h) / p. (= round(diff / p))
        uint64_t ratio = (diff + p_h) / p;

        // result = value + (ratio * p).
        uint64_t result = value + (ratio * p);

        return result;
    }
}