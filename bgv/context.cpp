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

        // ��ⷯ�� ü���� ������� �Ķ���� ����.
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

        // �ִ� ���� ���� (X�� ǥ�����ڰ� 3.2�� ����þ� �������� ���� ���̴�. ���� X�� 10���� ����Ѵ�.)
        // ���� ���� ���� ���: �� p * (X^2*d + X^2*d + X) + m
        // ���� ������ �� ���: �� p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) + m^2
        // ���� p��� q�� ũ�⸦ (p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) * p^2) �̻����� ������ ����ϴ�.
        uint64_t max_cipher_value = (p * p) * (
            + (static_cast<uint64_t>(40000) * d * d * d)
            + (static_cast<uint64_t>(4000) * d * d)
            + (static_cast<uint64_t>(500) * d)
            + 20
            + 1);
        uint64_t max_cipher_value_bit = static_cast<uint64_t>(std::log2l(max_cipher_value)) + static_cast<uint64_t>(1);

        // q�� Ȧ���� Zq���� [0, q/2]�� ���, [q/2+1, q-1]�� Ȧ���� ��Ÿ����.
        // ���� �� |m + pe|�� q/2�� �Ѱ� �Ǹ� �߸��� ���� ������ �ȴ�.
        // ���� ��� q�� 13�̰� m + pe�� 8�� ���, -5 ����� �Ǿ� �ٸ� ���� ������ �ȴ�. �̴� ������ ���� �Ȱ��� ������ �ȴ�.
        // ���� |ct.coeff| < q/2�� �ǹ̴�, ����� [0, q/2] ���� ����, ������ [q/2+1, q-1] ���� ���� �׽� �����ϰ� �ϱ� ���� �ʿ� �����̴�.
        uint64_t q_bound = static_cast<uint64_t>(1) << (max_cipher_value_bit + 1);

        // ���� ������ �����ϴ� q�� (dep + 1)�� ã�´�.
        // 1) q[i+1] > q[i] > p
        // 2) q[i+1] = q[i] = 1 mod p
        // 3) q�� Ȧ��
        // 4) q�� p�� ���μ�
        // ���� k * p + 1���� ���� 2�� 4�� �׻� �����ϴ� ���� ���� �� ������, k�� �׻� ¦���� �����ϸ� ���� 3���� �׻� ������ų �� �ִ�.
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