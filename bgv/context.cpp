#include "context.h"
#include <stdexcept>

namespace fheprac
{
    Context::Context(const uint64_t poly_modulus_degree, const uint64_t plain_modulus_bit_size, const uint64_t depth)
    {
        if (poly_modulus_degree == 0 || (poly_modulus_degree & (poly_modulus_degree - 1)) != 0) 
        {
            throw std::invalid_argument("The poly modulus degree must be a power of 2");
        }

        if (plain_modulus_bit_size < 2)
        {
            throw std::invalid_argument("The plain modulus bit size must be at least 2.");
        }

        if (depth < 1)
        {
            throw std::invalid_argument("The depth must be at least 1.");
        }

        poly_modulus_degree_ = poly_modulus_degree;

        plain_modulus_ = get_p(poly_modulus_degree, plain_modulus_bit_size);

        depth_ = depth;

        // ��ⷯ�� ü���� ������� �Ķ���� ����.
        for (const auto& q : create_modulus_chain())
        {
            params_.push_back(EncryptionParameters(q, params_.size(), (params_.size() + depth_ - 1) % depth_));
        }
    }

    uint64_t Context::poly_modulus_degree() const
    {
        return poly_modulus_degree_;
    }

    uint64_t Context::plain_modulus_value() const
    {
        return plain_modulus_;
    }

    uint64_t Context::depth() const
    {
        return depth_;
    }

    uint64_t Context::slot_count() const
    {
        return poly_modulus_degree_;
    }

    EncryptionParameters Context::first_param() const
    {
        return params_.back();
    }

    EncryptionParameters Context::last_param() const
    {
        return params_.front();
    }

    EncryptionParameters Context::param(const uint64_t index) const
    {
        return params_[index];
    }

    uint64_t Context::get_p(const uint64_t factor, const uint64_t bit_size) const
    {
        // (2^bit_size - 1) / factor * factor + 1 ���� factor�� ���鼭 �Ҽ��� ã��.
        uint64_t candidate = ((static_cast<uint64_t>(1) << bit_size) - 1) / factor * factor + 1;
        uint64_t lower_bound = static_cast<uint64_t>(1) << (bit_size - 1);
        bool found = false;

        while (candidate > lower_bound)
        {
            seal::Modulus new_mod(candidate);

            if (new_mod.is_prime())
            {
                found = true;
                break;
            }

            candidate -= factor;
        }

        if (found == false)
        {
            throw std::logic_error("failed to find enough qualifying primes");
        }

        return candidate;
    }

    uint64_t Context::get_q(const uint64_t factor, const uint64_t lower_bound) const
    {
        // (2^bit_size - 1) / factor * factor + 1 ���� factor�� ���ϸ鼭 �Ҽ��� ã��.
        uint64_t candidate = ((lower_bound + factor) / factor) * factor + 1;
        uint64_t upper_bound = static_cast<uint64_t>(1) << (64 - 1);
        bool found = false;

        while (candidate < upper_bound)
        {
            seal::Modulus new_mod(candidate);

            if (new_mod.is_prime())
            {
                found = true;
                break;
            }

            candidate += factor;
        }

        if (found == false)
        {
            throw std::logic_error("failed to find enough qualifying primes");
        }

        return candidate;
    }

    std::vector<uint64_t> Context::create_modulus_chain() const
    {
        uint64_t depth = depth_;
        uint64_t p = plain_modulus_;
        uint64_t d = poly_modulus_degree_;

        // �ִ� ���� ���� (X�� ǥ�����ڰ� 3.2�� ����þ� �������� ���� ���̴�. ���� X�� 10���� ����Ѵ�.)
        // ���� ���� ���� ���: �� p * (X^2*d + X^2*d + X) + m
        // ���� ������ �� ���: �� p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) + m^2
        // 
        // Modulus switching�� closest �̵����� ���� ���� ��. (�̶�, mod switch Ƚ���� ����Ͽ� depth - 1�� ���Ѵ�.)
        // (p / 2) * d * (10 * d) * (depth - 1)
        // 
        // 
        // ���� p��� q�� ũ�⸦ (p^2 * (4*X^4*d^3 + 4*X^3*d^2 + 5*X^2*d + 2*X) * p^2) + (p / 2) * d * (10 * d) * (depth - 1) �̻����� ������ ����ϴ�.
        uint64_t max_cipher_value =
            ((p * p) * ((static_cast<uint64_t>(40000) * d * d * d) + (static_cast<uint64_t>(4000) * d * d) + (static_cast<uint64_t>(500) * d) + 20 + 1))
            + ((p >> static_cast <uint64_t>(1)) * d * (10 * d) * depth)
            + (p * 600 * d * d * depth);
            
        uint64_t max_cipher_value_bit = static_cast<uint64_t>(std::log2l(max_cipher_value)) + static_cast<uint64_t>(1);

        // q�� Ȧ���� Zq���� [0, q/2]�� ���, [q/2+1, q-1]�� Ȧ���� ��Ÿ����.
        // ���� �� |m + pe|�� q/2�� �Ѱ� �Ǹ� �߸��� ���� ������ �ȴ�.
        // ���� ��� q�� 13�̰� m + pe�� 8�� ���, -5 ����� �Ǿ� �ٸ� ���� ������ �ȴ�. �̴� ������ ���� �Ȱ��� ������ �ȴ�.
        // ���� |ct.coeff| < q/2�� �ǹ̴�, ����� [0, q/2] ���� ����, ������ [q/2+1, q-1] ���� ���� �׽� �����ϰ� �ϱ� ���� �ʿ� �����̴�.
        uint64_t q_bound = static_cast<uint64_t>(1) << (max_cipher_value_bit + 1);

        // ���� ������ �����ϴ� q�� (depth + 1)�� ã�´�.
        // 1) q[i+1] > q[i] > p
        // 2) q[i+1] = q[i] = 1 mod p
        // 3) q�� �Ҽ�(or Ȧ��)
        // 4) q�� p�� ���μ�
        // ���� k * p + 1���� ���� 2�� 4�� �׻� �����ϴ� ���� ���� �� �ִ�.
        // ���� k���� ������ �����鼭 �Ҽ����� �Ǻ��ϸ� �Ҽ� ���ǵ� ���� ��ų �� �ִ�.
        std::vector<uint64_t> q_chain(depth + 1);

        q_chain[0] = get_q(p, q_bound);

        for (uint64_t i = 1; i < q_chain.size(); i++)
        {
            q_chain[i] = get_q(p, q_chain[i - 1] * p);
        }

        return q_chain;
    }
}