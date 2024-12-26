#include "context.h"
#include <seal/seal.h>

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
        for (const auto& q : create_modulus_chain(plain_mod_, dep_))
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

    std::vector<uint64_t> Context::create_modulus_chain(uint64_t t, uint64_t l) const
    {
        std::vector<uint64_t> q;

        // t보다 큰 서로소 소수 L + 1개를 찾는다.
        l = l + 1;
        uint64_t start = std::max(static_cast<uint64_t>(2), t + 1);

        // 공개키 생성, 암호화, 복호화 과정에서 대략 누적 오차가 약 p*(e^3)이 발생한다. 따라서 넉넉하게 p대비 q의 크기를 10비트 이상으로 잡는다.  
        uint64_t start_bit = static_cast<uint64_t>(std::log2(start)) + 1;
        uint64_t min_val_with_5_bits = static_cast<uint64_t>(1) << (start_bit + 10);
        start = std::max(start, min_val_with_5_bits);

        q.reserve(l);

        for (uint64_t i = 0; i < l; i++)
        {
            uint64_t prime = find_relatively_prime(start, t);
            q.push_back(prime);
            start = prime + 1;
        }

        std::reverse(q.begin(), q.end());
        
        return q;
    }
}