#include "context.h"
#include <seal/seal.h>
#include <stdexcept>
#include <iostream>

namespace fheprac
{
    Context::Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t depth)
    {
        deg_ = poly_modulus_degree;

        plain_mod_ = get_p(poly_modulus_degree, plain_modulus_bit_size);

        dep_ = depth;

        n_ = 1;

        N_ = 1;

        // 모듈러스 체인을 기반으로 파라미터 생성.
        for (const auto& q : create_modulus_chain())
        {
            params_.push_back(EncryptionParameters(q, params_.size(), (params_.size() + dep_ - 1) % dep_));
        }
    }

    uint64_t Context::poly_modulus_degree() const
    {
        return deg_;
    }

    uint64_t Context::plain_modulus_value() const
    {
        return plain_mod_;
    }

    uint64_t Context::depth() const
    {
        return dep_;
    }

    EncryptionParameters Context::first_param() const
    {
        return params_.back();
    }

    EncryptionParameters Context::last_param() const
    {
        return params_.front();
    }

    EncryptionParameters Context::param(uint64_t index) const
    {
        return params_[index];
    }

    uint64_t Context::get_p(const uint64_t factor, const int bit_size) const
    {
        // Start with (2^bit_size - 1) / factor * factor + 1
        uint64_t value = ((uint64_t(0x1) << bit_size) - 1) / factor * factor + 1;
        uint64_t lower_bound = uint64_t(0x1) << (bit_size - 1);
        bool found = false;

        while (value > lower_bound)
        {
            seal::Modulus new_mod(value);

            if (new_mod.is_prime())
            {
                found = true;
                break;
            }

            value += factor;
        }

        if (found == false)
        {
            throw std::logic_error("failed to find enough qualifying primes");
        }

        return value;
    }

    uint64_t Context::get_q(const uint64_t factor, const uint64_t lower_bound) const
    {
        uint64_t candidate = ((lower_bound + factor) / factor) * factor + 1;

        if (!(candidate % 2))
        {
            candidate += factor;
        }

        return candidate;
    }

    std::vector<uint64_t> Context::create_modulus_chain() const
    {
        uint64_t dep = dep_;
        uint64_t p = plain_mod_;
        uint64_t d = deg_;

        // 최대 누적 오차 (X는 표준편자가 3.2인 가우시안 분포에서 뽑은 값이다. 따라서 X는 10으로 계산한다.)
        // 연산 하지 않은 경우: 약 p * (X^2*d + X^2*d + X) + m
        // 곱셈 연산을 한 경우: 약 p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) + m^2
        // 따라서 p대비 q의 크기를 (p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) * p^2) 이상으로 잡으면 충분하다.
        uint64_t max_cipher_value = (p * p) * (
            + (static_cast<uint64_t>(40000) * d * d * d)
            + (static_cast<uint64_t>(4000) * d * d)
            + (static_cast<uint64_t>(500) * d)
            + 20
            + 1);
        uint64_t max_cipher_value_bit = static_cast<uint64_t>(std::log2l(max_cipher_value)) + static_cast<uint64_t>(1);

        // q가 홀수인 Zq에서 [0, q/2]은 양수, [q/2+1, q-1]은 홀수를 나타낸다.
        // 연산 후 |m + pe|가 q/2를 넘게 되면 잘못된 값이 나오게 된다.
        // 예를 들어 q가 13이고 m + pe가 8인 경우, -5 취급이 되어 다른 값이 나오게 된다. 이는 음수일 때도 똑같은 문제가 된다.
        // 따라서 |ct.coeff| < q/2의 의미는, 양수는 [0, q/2] 범위 내에, 음수는 [q/2+1, q-1] 범위 내에 항시 존재하게 하기 위해 필요 조건이다.
        uint64_t q_bound = static_cast<uint64_t>(1) << (max_cipher_value_bit + 1);

        // 다음 조건을 만족하는 q를 (dep + 1)개 찾는다.
        // 1) q[i+1] > q[i] > p
        // 2) q[i+1] = q[i] = 1 mod p
        // 3) q는 홀수
        // 4) q와 p는 서로소
        // 수식 k * p + 1으로 조건 2와 4를 항상 만족하는 값을 구할 수 있으며, k를 항상 짝수로 설정하면 조건 3또한 항상 만족시킬 수 있다.
        std::vector<uint64_t> q_chain(dep + 1);
        uint64_t q0 = get_q(p, q_bound);
        uint64_t q1 = get_q(p, p);

        q_chain[0] = q0;

        for (uint64_t i = 1; i < q_chain.size(); i++)
        {
            q_chain[i] = q_chain[i - 1] * q1;
        }

        // TEST
        std::cout << max_cipher_value_bit << '\n';
        std::cout << max_cipher_value << '\n';
        std::cout << q_bound << '\n';
        std::cout << q0 << '\n';
        std::cout << q1 << '\n';

        return q_chain;
    }
}