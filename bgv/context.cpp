#include "context.h"
#include <seal/seal.h>
#include <stdexcept>

namespace fheprac
{
    Context::Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t depth)
    {
        deg_ = poly_modulus_degree;

        plain_mod_ = seal::PlainModulus::Batching(poly_modulus_degree, plain_modulus_bit_size).value();

        dep_ = depth;

        n_ = 1;

        N_ = 1;

        // 모듈러스 체인 기반으로 파라미터 생성.
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

    uint64_t Context::find_odd_coprime_to_prime(uint64_t bound, uint64_t rp_factor) const
    {
        uint64_t candidate = (bound % 2 == 0) ? bound + 1 : bound;

        while (true)
        {
            if (candidate % rp_factor != 0)
            {
                return candidate;
            }
            candidate += 2;
        }
    }

    std::vector<uint64_t> Context::create_modulus_chain() const
    {
        uint64_t dep = dep_;
        uint64_t p = plain_mod_;
        uint64_t d = deg_;

        // 암호화, 복호화 과정에서 대략 누적 오차가 약 p * (d*X) * (2*X+1) = p * (2*d*X^2 + d*X)이 발생한다.
        // X는 표준편자가 3.2인 가우시안 분포에서 뽑은 값이다. 따라서 X는 10으로 계산한다.
        // 따라서 p대비 q_chain의 크기를 (p * (2*d*X^2 + d*X))비트 보다 크게 잡으면 충분하다. 
        uint64_t expected = p * ((static_cast<uint64_t>(2) * d * static_cast<uint64_t>(100)) + (d * static_cast<uint64_t>(10)) + static_cast<uint64_t>(1));
        uint64_t expected_bit = static_cast<uint64_t>(std::log2l(expected)) + static_cast<uint64_t>(1);
        uint64_t q_bound = static_cast<uint64_t>(1) << (expected_bit + 1);

        // p와 서로소인 홀수를 (dep + 1)개 찾는다.
        std::vector<uint64_t> q_chain(dep + 1);

        for (uint64_t i = 0; i < q_chain.size(); i++)
        {
            uint64_t q = find_odd_coprime_to_prime(q_bound, p);
            q_chain[i] = q;
            q_bound = q + 2;
        }

        return q_chain;
    }
}