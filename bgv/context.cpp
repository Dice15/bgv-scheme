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

        // ��ⷯ�� ü�� �� �Ķ���� ����
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

    // ����
    // 1) �缱��ȭ Ű ����. �̶� ���� sk�� ����ϵ��� ��
    // 2) ���� ����
    // 3) ���� ����
    //  
    // ����ȭ
    // 1) q�� ���� ���ϴ� �� 
    // 2) ������ ���� ����
    // 3) ���׽� �������� NTTȰ���ϵ��� ��
    //
    std::vector<uint64_t> Context::create_modulus_chain() const
    {
        uint64_t dep = dep_;
        uint64_t p = plain_mod_;
        uint64_t p_bit = static_cast<uint64_t>(std::log2(p)) + static_cast<uint64_t>(1);

        // ��ȣȭ, ��ȣȭ �������� �뷫 ���� ������ �� p * (6 * X^2 + 30)�� �߻��Ѵ�.
        // X�� ǥ�����ڰ� 3.2�� ����þ� �����̴�.
        // ���� p��� q�� ũ�⸦ 11��Ʈ �̻����� ������ ����ϴ�.  
        uint64_t q_bound = static_cast<uint64_t>(1) << (p_bit + 11);

        // p���� ū ���μ� �Ҽ� dep + 1���� ã�´�.
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