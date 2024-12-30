#include "context.h"
#include <seal/seal.h>
#include <stdexcept>
#include <iostream>

namespace fheprac
{
    Context::Context(uint64_t poly_modulus_degree, uint64_t plain_modulus_bit_size, uint64_t depth)
    {
        deg_ = poly_modulus_degree;

        plain_mod_ = get_prime(poly_modulus_degree, plain_modulus_bit_size);

        dep_ = depth;

        n_ = 1;

        N_ = 1;

        // ��ⷯ�� ü�� ������� �Ķ���� ����.
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

    uint64_t Context::get_prime(const uint64_t factor, const int bit_size) const
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

    uint64_t Context::get_relatively_prime_odd(const uint64_t bound, const uint64_t rp_factor) const
    {
        uint64_t candidate = (bound % 2 == 0) ? bound + 1 : bound;

        while (true)
        {
            if (candidate % rp_factor == 1)
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

        // �ִ� ���� ���� (X�� ǥ�����ڰ� 3.2�� ����þ� �������� ���� ���̴�. ���� X�� 10���� ����Ѵ�.)
        //    - �⺻: �� p * (X^2*d + X^2*d + X)�� �߻��Ѵ�.
        //    - ����: �� p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X)
        // ���� p��� q_chain�� ũ�⸦ (p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X + 1))���� ũ�� ������ ����ϴ�.
        uint64_t expected_c_size = (p * p) * (
            + (static_cast<uint64_t>(40000) * d * d * d)
            + (static_cast<uint64_t>(4000) * d * d)
            + (static_cast<uint64_t>(500) * d)
            + 20
            + 1);

        uint64_t expected_c_size_bit = static_cast<uint64_t>(std::log2l(expected_c_size)) + static_cast<uint64_t>(1);
        uint64_t q_bound = static_cast<uint64_t>(1) << (expected_c_size_bit + 2);

        //std::cout << expected_c_size_bit << '\n';
        //std::cout << expected_c_size << '\n';
        //std::cout << q_bound << '\n';

        // p�� ���μ��� Ȧ���� (dep + 1)�� ã�´�.
        std::vector<uint64_t> q_chain(dep + 1);

        for (uint64_t i = 0; i < q_chain.size(); i++)
        {
            uint64_t q = get_relatively_prime_odd(q_bound, p);
            q_chain[i] = q;
            q_bound = q + 1;
        }

        return q_chain;
    }
}