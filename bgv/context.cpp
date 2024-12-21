#include "context.h"
#include <ranges>
#include <seal/seal.h>

namespace fheprac
{
	Context::Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t level)
	{
		d_ = poly_modulus_degree;

		t_ = seal::PlainModulus::Batching(poly_modulus_degree, plain_modulus_bit_size).value();

        l_ = level;

		n_ = 1;

		N_ = 1;
	
        // x를 표준편차가 3.2인 가우시안 분포로 설정.
		std::random_device rd;
		rand_.seed(rd());
        gaussian_dist_ = std::normal_distribution<double_t>(0.0, 3.2);

        // 모듈러스 체인 및 파라미터 생성
        for (const auto& q : create_modulus_chain(t_, l_))
        {
            params_.push_back(EncryptionParameters(q, params_.size(), (params_.size() + l_ - 1) % l_));
        }
	}
    
    uint64_t Context::poly_modulus_degree() const
    {
        return d_;
    }

    uint64_t Context::plain_modulus_value() const
    {
        return t_;
    }

    uint64_t Context::level() const
    {
        return l_;
    }

    uint64_t Context::value_from_gaussian_dist()
    {
        // 분포에서 뽑은 값이 음수일 수 있으므로 절대값을 취해준다.
        return static_cast<uint64_t>(std::abs(std::llround(gaussian_dist_(rand_))));
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
        std::vector<uint64_t> p;   // prime factors
        std::vector<uint64_t> q;

        // t보다 큰 서로소 소수 L + 1개를 찾는다.
        l = l + 1;
        uint64_t start = std::max(static_cast<uint64_t>(2), t + 1);

        p.reserve(l);

        for (uint64_t i = 0; i < l; i++)
        {
            uint64_t prime = find_relatively_prime(start, t);
            p.push_back(prime);
            start = prime + 1;
        }

        // L개의 q_i계산
        // q_L = p_1 * p_2 * ... * p_L
        // q_{L-1} = p_1 * ... * p_{L-1}
        // ...
        // q_1 = p_1

        q.reserve(l);
        q.push_back(p[0]);

        for (uint64_t i = 1; i < l; i++)
        {
            q.push_back(q.back() * p[i]);
        }
        
        return q;
    }
}