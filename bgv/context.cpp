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

        // 모듈러스 체인 및 파라미터 생성
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

    bool Context::is_prime(uint64_t val) const
    {
        if (val < 2) return false;
        if (val % 2 == 0 && val > 2) return false;

        for (uint64_t i = 3; i <= static_cast<uint64_t>(std::sqrt(static_cast<double_t>(val))); i += 2)
        {
            if (val % i == 0) return false;
        }

        return true;
    }

    uint64_t Context::find_relatively_prime(uint64_t min_val, uint64_t rp_factor) const
    {
        uint64_t candidate = min_val;

        while (true)
        {
            if (is_prime(candidate) && std::gcd(candidate, rp_factor) == 1)
            {
                return candidate;
            }
            candidate++;
        }
    }

    // 구현
    // 1) 재선형화 키 생성. 이때 같은 sk를 사용하도록 함
    // 2) 덧셈 구현
    // 3) 곱셈 구현
    //  
    // 최적화
    // 1) q를 빨리 구하는 법 
    // 2) 안전한 곱셈 개선
    // 3) 다항식 곱셈에서 NTT활용하도록 함
    //
    std::vector<uint64_t> Context::create_modulus_chain() const
    {
        uint64_t dep = dep_;
        uint64_t p = plain_mod_;
        uint64_t p_bit = static_cast<uint64_t>(std::log2(p)) + static_cast<uint64_t>(1);

        // 암호화, 복호화 과정에서 대략 누적 오차가 약 p * (6 * X^2 + 30)이 발생한다.
        // X는 표준편자가 3.2인 가우시안 분포이다.
        // 따라서 p대비 q의 크기를 11비트 이상으로 잡으면 충분하다.  
        uint64_t q_bound = static_cast<uint64_t>(1) << (p_bit + 11);

        // p보다 큰 서로소 소수 dep + 1개를 찾는다.
        std::vector<uint64_t> q_chain;

        q_chain.reserve(dep + 1);

        for (uint64_t i = 0; i <= dep; i++)
        {
            uint64_t q = find_relatively_prime(q_bound, p);
            q_chain.push_back(q);
            q_bound = q + 1;
        }

        if (static_cast<uint64_t>(q_chain.size()) < dep + 1)
        {
            throw std::invalid_argument("The number of q values is less than dep + 1.");
        }

        std::reverse(q_chain.begin(), q_chain.end());

        return q_chain;
    }
}