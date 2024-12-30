#include "evaluator.h"
#include "encryptionparams.h"
#include <stdexcept>
#include <iostream>
#include <intrin.h>

namespace fheprac
{
    Evaluator::Evaluator(const Context& context) :context_(context) {}

    void Evaluator::mod_switch(const Ciphertext& ciphertext, Ciphertext& destination) const
    {
        const EncryptionParameters& curr_params = ciphertext.params();
        const EncryptionParameters& next_params = context_.param(curr_params.next_param_index());

        if (curr_params.l() <= next_params.l())
        {
            throw std::out_of_range("Modulus switching cannot proceed further.");
        }

        const uint64_t d = context_.poly_modulus_degree();
        const uint64_t p = context_.plain_modulus_value();
        const uint64_t curr_q = curr_params.q();
        const uint64_t next_q = next_params.q();

        // ct, ct': 암호문 데이터. (2x1 poly matrix)
        const PolyMatrix& ct = ciphertext.data();
        PolyMatrix ct_dot(ct.row_size(), ct.col_size(), d - 1, next_q);

        // curr_q > next_q > p and curr_q = next_q = 1 mod p.
        // [<ct', s>]_next_q = [<ct, s>]_curr_q mod p.
        // ct' = closest((curr_q / next_q) * ct) mod p.
        for (uint64_t row = 0; row < ct_dot.row_size(); row++)
        {
            for (uint64_t col = 0; col < ct_dot.col_size(); col++)
            {
                for (uint64_t i = 0; i <= ct_dot.degree(); i++)
                {
                    const uint64_t coeff_curr_q = ct.get(row, col, i);
                    const uint64_t coeff_next_q = drop_to_next_q(coeff_curr_q, curr_q, next_q, p);
                    ct_dot.set(row, col, i, coeff_next_q);
                }
            }
        }

        destination.data(ct_dot);
        destination.params(next_params);
    }

    void Evaluator::relinearize(const Ciphertext& ciphertext, RelinKeys& relinKeys, Ciphertext& destination) const
    {
        if (ciphertext.size() != static_cast<size_t>(3))
        {
            throw std::out_of_range(".");    // 오직 크기가 3인 암호문만 재선형화가 가능합니다.
        }

        const EncryptionParameters& params = ciphertext.params();
        const uint64_t d = context_.poly_modulus_degree();
        const uint64_t l = params.l();
        const uint64_t q = params.q();
        const uint64_t log_q = static_cast<uint64_t>(std::ceill(std::log2l(params.q())));
        const uint64_t N = ciphertext.size() * log_q;

        // ct: 암호문 데이터. (3x1 poly matrix)
        const PolyMatrix& ct = ciphertext.data();

        // D: 암호문 비트. (Nx1 poly matrix)
        PolyMatrix D;
        D.assign(N, 1, d - 1, q);

        for (size_t r = 0; r < ct.row_size(); r++)
        {
            for (size_t c = 0, bit = 1; c < log_q; c++, bit <<= 1)
            {
                for (size_t i = 0; i <= ct.degree(); i++)
                {
                    D.set((r * log_q) + c, 0, i, (ct.get(r, 0, i) & bit) ? 1 : 0);
                }
            }
        }

        // rk: 재선형화키. (Nx2 poly matrix)
        const PolyMatrix& rk = relinKeys.data(l);

        // ct_2: 재선형화가 완료된 암호문. (2x1 poly matrix)
        destination.data((D.t() * rk).t());
        destination.params(params);
    }

    void Evaluator::add(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const
    {
        if (ciphertext1.params().l() != ciphertext2.params().l())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 parameter mismatch.");
        }

        if (ciphertext1.size() != ciphertext2.size())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 size mismatch.");
        }

        destination.data(ciphertext1.data() + ciphertext2.data());
        destination.params(ciphertext1.params());
    }

    void Evaluator::sub(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const
    {
        if (ciphertext1.params().l() != ciphertext2.params().l())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 parameter mismatch.");
        }

        if (ciphertext1.size() != ciphertext2.size())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 size mismatch.");
        }

        destination.data(ciphertext1.data() - ciphertext2.data());
        destination.params(ciphertext1.params());
    }

    void Evaluator::multiply(const Ciphertext& ciphertext1, const Ciphertext& ciphertext2, Ciphertext& destination) const
    {
        if (ciphertext1.params().l() != ciphertext2.params().l())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 parameter mismatch.");
        }

        if (ciphertext1.size() != ciphertext2.size())
        {
            throw std::out_of_range("ciphertext1 and ciphertext2 size mismatch.");
        }

        if (ciphertext1.size() != static_cast<size_t>(2))
        {
            throw std::out_of_range("Only 2 size of ciphertext can multiply."); // 오직 크기가 2인 암호문만 곱셈연산이 가능합니다.
        }

        const EncryptionParameters& params = ciphertext1.params();
        const uint64_t d = context_.poly_modulus_degree();
        const uint64_t q = params.q();

        PolyMatrix ct(ciphertext1.size() + 1, 1, d - 1, q);
        const Polynomial& c0 = ciphertext1.data().get(0, 0);
        const Polynomial& c1 = ciphertext1.data().get(1, 0);
        const Polynomial& k0 = ciphertext2.data().get(0, 0);
        const Polynomial& k1 = ciphertext2.data().get(1, 0);

        ct.set(0, 0, c0 * k0);
        ct.set(1, 0, (c0 * k1) + (c1 * k0));
        ct.set(2, 0, c1 * k1);

        destination.data(ct);
        destination.params(ciphertext1.params());
    }

    uint64_t Evaluator::drop_to_next_q(const uint64_t value, const uint64_t curr_q, const uint64_t next_q, const uint64_t p) const
    {
        const uint64_t curr_q_h = curr_q >> 1;
        const uint64_t p_h = p >> 1;
   
        /*
        * 계산 정확도를 위해 소수점을 사용하지 않고 계산.
        * Origin code. (using point)
        uint64_t scaled = {(value * next_q) + (curr_q / 2)} / curr_q;
        */

        uint64_t high1, low1, carry, remainder;

        // scaled = {(value * next_q) + (curr_q / 2)} / curr_q.
        low1 = _umul128(value, next_q, &high1);
        carry = (low1 + curr_q_h < low1);
        low1 += curr_q_h;
        high1 += carry;
        uint64_t scaled = _udiv128(high1, low1, curr_q, &remainder);

        return scaled;
    }
}