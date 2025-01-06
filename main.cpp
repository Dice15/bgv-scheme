#include "modules/fhe-prac/bgv/ciphertext.h"
#include "modules/fhe-prac/bgv/context.h"
#include "modules/fhe-prac/bgv/decryptor.h"
#include "modules/fhe-prac/bgv/encoder.h"
#include "modules/fhe-prac/bgv/encryptor.h"
#include "modules/fhe-prac/bgv/evaluator.h"
#include "modules/fhe-prac/bgv/keygenerator.h"
#include "modules/fhe-prac/bgv/plaintext.h"
#include "modules/fhe-prac/bgv/publickey.h"
#include "modules/fhe-prac/bgv/relinkeys.h"
#include "modules/fhe-prac/bgv/secretkey.h"
#include "modules/fhe-prac/bgv/util/polynomial.h"
#include "modules/random/randomgenerator.h"
#include <iostream>
#include <stdexcept>
#include <vector>

// Completed
// 1) 비밀키, 공개키, 재선형화키 생성
// 2) 암호문 및 평문의 덧셈/뺄셈/곱셈
// 3) 재선형화
// 4) 모듈러스 스위칭

// TODO
// 1) CRT를 활용한 NTT 적용

void my_bgv_test_basic()
{
    // Create Context
    std::cout << "\\n========================== Context ==========================\n";

    fheprac::Context context(8, 8, 2);

    std::cout << "\npoly modulus degree: " << context.poly_modulus_degree() << '\n';
    std::cout << "\nplain modulus value: " << context.plain_modulus_value() << '\n';
    std::cout << "\ndepth: " << context.depth() << '\n';
    std::cout << "\nparam" << '\n';

    for (size_t j = 0; j <= context.depth(); j++)
    {
        auto param = context.param(j);
        std::cout << "level: " << param.l() << ", next level: " << param.next_param_index() << ", q: " << param.q() << '\n';
    }


    // Create Key Generator
    std::cout << "\n\n========================== KeyGenerator ==========================\n";

    fheprac::KeyGenerator key_gen(context);
    fheprac::SecretKey sk;
    fheprac::PublicKey pk;
    fheprac::RelinKeys rk;

    sk = key_gen.secret_key();
    key_gen.create_public_key(pk);
    key_gen.create_relin_keys(rk);


    // Create Encoder, Encryptor, Decryptor
    std::cout << "\n\n========================== Encoder, Encryptor, Decryptor ==========================\n";

    fheprac::Encoder encoder(context);
    fheprac::Encryptor encryptor(context, pk);
    fheprac::Decryptor decryptor(context, sk);

    std::vector<int64_t> v1(context.poly_modulus_degree());
    std::vector<int64_t> v2(context.poly_modulus_degree());
    std::vector<int64_t> v3(context.poly_modulus_degree());
    std::vector<int64_t> v_result;

    fheprac::Plaintext p1;
    fheprac::Plaintext p2;
    fheprac::Plaintext p3;
    fheprac::Plaintext p_result;

    fheprac::Ciphertext c1;
    fheprac::Ciphertext c2;
    fheprac::Ciphertext c3;
    fheprac::Ciphertext c_result;

    for (size_t i = 0; i < context.poly_modulus_degree(); i++)
    {
        v1[i] = 60 + i;      // [60, 61, 62, 63, 64, 65, 66, 67]
        v2[i] = i ? 0 : 2;   // [ 2,  0,  0,  0,  0,  0,  0,  0]
        v3[i] = -(10 + i);   // -[10, 11, 12, 13, 14, 15, 16, 17]
    }

    encoder.encode(v1, p1);
    encoder.encode(v2, p2);
    encoder.encode(v3, p3);

    encryptor.encrypt(p1, c1);
    encryptor.encrypt(p2, c2);
    encryptor.encrypt(p3, c3);


    // Test Evaluator's Add, Sub, Multiply
    std::cout << "\n\n========================== Evaluator's Add, Sub, Multiply ==========================\n";

    fheprac::Evaluator evaluator(context);

    // Expected result: [80, 88, 96, 104, 112, 120, 128, 136] mod p -> [80, 88, 96, 104, 112, 120, 128, -121]
    evaluator.add(c1, c1, c_result);
    evaluator.add(c_result, p1, c_result);
    evaluator.sub(c_result, p1, c_result);
    evaluator.sub(c_result, p1, c_result);
    evaluator.sub(c_result, p1, c_result);
    evaluator.sub(c_result, c3, c_result);
    evaluator.multiply(c_result, p2, c_result);
    evaluator.multiply(c_result, c2, c_result);
    evaluator.multiply(c_result, p2, c_result);

    decryptor.decrypt(c_result, p_result);
    encoder.decode(p_result, v_result);

    for (size_t i = 0; i < context.poly_modulus_degree(); i++)
    {
        int64_t ans = (-v3[i]) * 8;

        if (ans > context.plain_modulus_value() / 2)
        {
            ans -= context.plain_modulus_value();
        }

        std::cout << v_result[i] << '\n';

        if (v_result[i] != ans)
        {
            throw std::logic_error("Wrong Answer");
        }
    }


    // Test Evaluator's Relinearize, Modulus Switching
    std::cout << "\n\n========================== Evaluator's Relinearize, Modulus Switching ==========================\n";
    std::cout << "l = " << context.depth() << " -> " << context.depth() - 1 << '\n';

    evaluator.relinearize(c_result, rk, c_result);
    evaluator.mod_switch(c_result, c_result);
    decryptor.decrypt(c_result, p_result);
    encoder.decode(p_result, v_result);

    for (size_t i = 0; i < context.poly_modulus_degree(); i++)
    {
        int64_t ans = (-v3[i]) * 8;

        if (ans > context.plain_modulus_value() / 2)
        {
            ans -= context.plain_modulus_value();
        }

        if (v_result[i] != ans)
        {
            throw std::logic_error("Wrong Answer");
        }
    }
}

void my_bgv_test_binary_matching()
{
    // Bob Function
    const auto bob = [](
        const fheprac::Ciphertext& pattern_ct, 
        const size_t pattern_len, 
        const fheprac::PublicKey& pk, 
        const fheprac::RelinKeys& rk, 
        fheprac::Ciphertext& destination)
    {
        // Create BGV Scheme using received public data from Alice.
        fheprac::Context context(32, 6, 2);
        fheprac::Encoder encoder(context);
        fheprac::Encryptor encryptor(context, pk);
        fheprac::Evaluator evaluator(context);

        // Random Generator
        RandomGenerator rand;

        // Bob's data
        std::vector<int64_t> text_vt = {
            1, -1, -1, 1, 1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 1, 1, 1, -1, -1, -1, -1, 1, -1, 1, 1
        };
        std::vector<int64_t> len_vt(32, pattern_len);
        std::vector<int64_t> rand_vt(1, rand.get_integer<int64_t>(1, context.plain_modulus_value() / 2) * rand.get_integer<int64_t>({ -1, 1 }));

        // encrypt data
        fheprac::Plaintext text_pt;
        fheprac::Plaintext len_pt;
        fheprac::Plaintext rand_pt;

        fheprac::Ciphertext text_ct;
        fheprac::Ciphertext rand_ct;

        encoder.encode(text_vt, text_pt);
        encoder.encode(len_vt, len_pt);
        encoder.encode(rand_vt, rand_pt);

        encryptor.encrypt(text_pt, text_ct);
        encryptor.encrypt(rand_pt, rand_ct);

        // (text * pattern - pattern_len) * rand
        evaluator.multiply(text_ct, pattern_ct, destination);
        evaluator.relinearize(destination, rk, destination);
        evaluator.mod_switch(destination, destination);

        evaluator.sub(destination, len_pt, destination);

        evaluator.mod_switch(rand_ct, rand_ct);
        evaluator.multiply(destination, rand_ct, destination);
        evaluator.relinearize(destination, rk, destination);
        evaluator.mod_switch(destination, destination);
    };

    // Alice Function
    const auto alice = [&bob]()
    {
        // Create BGV Scheme using received public data from Alice.
        fheprac::Context context(32, 6, 2);
        fheprac::KeyGenerator key_gen(context);
        fheprac::SecretKey sk;
        fheprac::PublicKey pk;
        fheprac::RelinKeys rk;
        sk = key_gen.secret_key();
        key_gen.create_public_key(pk);
        key_gen.create_relin_keys(rk);
        fheprac::Encoder encoder(context);
        fheprac::Encryptor encryptor(context, pk);
        fheprac::Evaluator evaluator(context);
        fheprac::Decryptor decryptor(context, sk);

        // Alice's data
        std::vector<int64_t> pattern_vt = {
            1, -1, -1, -1, -1, -1, 1, -1, -1, 1
        };
        std::vector<int64_t> result_vt;

        std::reverse(pattern_vt.begin(), pattern_vt.end());

        // encrypt data
        fheprac::Plaintext pattern_pt;
        fheprac::Plaintext result_pt;

        fheprac::Ciphertext pattern_ct;
        fheprac::Ciphertext result_ct;

        encoder.encode(pattern_vt, pattern_pt);
        encryptor.encrypt(pattern_pt, pattern_ct);

        // send to bob
        bob(pattern_ct, 10, pk, rk, result_ct);

        // analyze result
        decryptor.decrypt(result_ct, result_pt);
        encoder.decode(result_pt, result_vt);

        for (size_t i = 0; i < 32; i++)
        {
            if (result_vt[i] == 0)
            {
                std::cout << "Found: " << (i + 1 - 10) << '\n';
            }
        }
    };

    alice();
}


int main()
{
    //my_bgv_test_basic();
    my_bgv_test_binary_matching();
    return 0;
}