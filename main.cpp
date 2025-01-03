﻿#include "examples.h"
#include "modules/io/teestream.h"
#include "modules/fhe/fhebuilder.h"
#include "modules/fhe/fhe.h"
#include "modules/random/randomgenerator.h"
#include "modules/simulator/patternmatch.h"
#include "modules/fhe-prac/bgv/context.h"
#include "modules/fhe-prac/bgv/keygenerator.h"
#include "modules/fhe-prac/bgv/secretkey.h"
#include "modules/fhe-prac/bgv/publickey.h"
#include "modules/fhe-prac/bgv/relinkeys.h"
#include "modules/fhe-prac/bgv/encoder.h"
#include "modules/fhe-prac/bgv/plaintext.h"
#include "modules/fhe-prac/bgv/ciphertext.h"
#include "modules/fhe-prac/bgv/encryptor.h"
#include "modules/fhe-prac/bgv/decryptor.h"
#include "modules/fhe-prac/bgv/evaluator.h"
#include "modules/fhe-prac/bgv/util/polynomial.h"
#include <intrin.h>
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <set>
#include <limits>
#include <fstream>
#include <sstream>
#include <iomanip> 
#include <future>
#include <thread>
#include <type_traits>

using namespace std;
using namespace seal;
using namespace fhe;

void integer_matching(const int32_t text_size = 2048, const int32_t pattern_size = 10, const int32_t unique_int_cnt = 4)
{
    // Create TeeStream instance inside the function
    TeeStream out("integer_matching_" + to_string(text_size) + "_" + to_string(pattern_size) + ".txt");

    out << endl << "------------------ <Testing integer matching: text size(" << text_size << "), pattern size(" << pattern_size << ")> ------------------" << endl << endl;

    // Generate random data
    RandomGenerator rand;

    set<int16_t> int_set = rand.get_integer_set<int16_t>(
        numeric_limits<int16_t>::min(),
        numeric_limits<int16_t>::max(),
        unique_int_cnt);

    vector<int16_t> text = rand.get_integer_vector<int16_t>(vector<int16_t>(int_set.begin(), int_set.end()), text_size);
    vector<int16_t> pattern = rand.get_integer_vector<int16_t>(vector<int16_t>(int_set.begin(), int_set.end()), pattern_size);

    for (auto& i : rand.get_integer_vector<int32_t>(0, text.size() - pattern.size(), rand.get_integer(3, 100)))
    {
        for (int32_t j = 0; j < pattern.size(); j++)
        {
            if (i + j < text.size())
            {
                text[i + j] = pattern[j];
            }
        }
    }

    // Testing
    PatternMatch simulator;

    {
        out << "\n- kmp\n";
        auto [time, matched] = simulator.integer_matching(text, pattern, integer_matching_type::kmp);
        if (matched.empty())
        {
            out << "\n    [ empty ]\n\n";
        }
        else
        {
            sort(matched.begin(), matched.end());

            out << "\n[ ";
            bool first = true;
            for (auto& e : matched)
            {
                if (!first) out << ", ";
                out << e;
                first = false;
            }
            out << " ]\n\n";
        }
        out << "    Execution time: " << time << "ms\n\n";
    }

    {
        out << "\n- hash + primitive root in bgv\n";
        auto [time, matched] = simulator.integer_matching(text, pattern, integer_matching_type::hash_primitive_root_in_bgv);
        if (matched.empty())
        {
            out << "\n    [ empty ]\n\n";
        }
        else
        {
            sort(matched.begin(), matched.end());

            out << "\n[ ";
            bool first = true;
            for (auto& e : matched)
            {
                if (!first) out << ", ";
                out << e;
                first = false;
            }
            out << " ]\n\n";
        }
        out << "    Execution time: " << time << "ms\n\n";
    }

    {
        out << "- hash + rotation in bgv\n";
        auto [time, matched] = simulator.integer_matching(text, pattern, integer_matching_type::hash_rotation_in_bgv);
        if (matched.empty())
        {
            out << "\n    [ empty ]\n\n";
        }
        else
        {
            sort(matched.begin(), matched.end());

            out << "\n[ ";
            bool first = true;
            for (auto& e : matched)
            {
                if (!first) out << ", ";
                out << e;
                first = false;
            }
            out << " ]\n\n";
        }
        out << "    Execution time: " << time << "ms\n\n";
    }
}

pair<int, vector<pair<int, int>>> count_pairs(long long target) {
    long long limit = static_cast<long long>(sqrt(target));
    int count = 0;
    vector<pair<int, int>> result;

    for (long long a = 1; a <= limit; ++a) {
        long long b_square = target - a * a;
        if (b_square < 0) break;

        long long b = static_cast<long long>(sqrt(b_square));
        if (b * b == b_square) {
            result.emplace_back(a, b);
            ++count;
        }
    }

    return { count, result };
}


// 안전한 모듈러 곱셈 함수
uint64_t safe_modular_multiplication(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 0;    // 결과값 초기화
    a %= mod;              // a를 mod로 나눈 나머지
    b %= mod;              // b를 mod로 나눈 나머지

    while (b > 0) {
        // b가 홀수라면 결과값에 a를 더함
        if (b & 1) {
            result = (result + a) % mod;
        }
        // a와 b를 2배씩 증가/감소
        a = (a << 1) % mod;
        b >>= 1;
    }

    return result;
}


// New Complete
// 1) 1개의 비밀키를 사용할 수 있도록 개선
// 2) 재선형화 구현
// 3) 모듈러스 스위칭 구현
// 4) 덧셈/뺄셈 구현
// 5) 곱셈 구현
// 
// Question
// 1) Zq에서 음수처리
// 2) 모듈러스 스위칭 c2 = (q2/q1) * c1 이렇게 하면 값이 이상하게 나옴
//  
// TODO
// 1) 안전한 곱셈 속도 개선
// 2) 다항식 곱셈에 NTT 적용
// 3) 나머지 연산 개선 (%연산자 사용하지 않기)
// 4) Zq에서 음수는 q/2초과의 수로 표현하기 때문에, m+pe가 q/2보다 커지는 현상이 발생. 
//    (음수는 공개키 생성 시, (Z[x]/x^d + 1) 위에서 곱셈할때 생김)
// 
// Total Complete
// 1) Keygenerator 구현. (secretkey/publickey/relinkey)
// 2) Encoder 구현. (encode, decode)
// 3) Encryptor 구현. (Encrypt)
// 4) Decryptor 구현. (Decrypt)
// 5) Evaluator 구현. (Modulus switch, Relinearize, Add, Sub, Multiply)
//

int main()
{
    cout << "\\n========================== Context ==========================\n";

    fheprac::Context context(8, 15, 2);

    cout << "\npoly modulus degree: " << context.poly_modulus_degree() << '\n';

    cout << "\nplain modulus value: "  << context.plain_modulus_value() << '\n';

    cout << "\ndepth: " << context.depth() << '\n';

    cout << "\nparam" << '\n';
    for (int j = context.depth(); j >= 0; j--)
    {
        auto param = context.param(j);
        cout << "level: " << param.l() << ", next level: " << param.next_param_index() << ", q: " << param.q() << '\n';
    }

    // key test
    cout << "\n\n========================== KeyGenerator ==========================\n";

    fheprac::KeyGenerator key_gen(context);
    fheprac::SecretKey sk;
    fheprac::PublicKey pk;
    fheprac::RelinKeys rk;

    sk = key_gen.secret_key();
    key_gen.create_public_key(pk);
    key_gen.create_relin_keys(rk);

   /* cout << "\nsecret key" << '\n';
    for (int j = context.depth(); j >= 0; j--)
    {
        auto sk_j = sk.data(j);

        cout << "\nl = " << j << ", dimension = " << sk_j.row_size() << " x " << sk_j.col_size() << '\n';

        for (int r = 0; r < sk_j.row_size(); r++)
        {       
            cout << (r == 0 ? "  1" : ("t^" + to_string(r))) << ": [ ";
            for (int c = 0; c < sk_j.col_size(); c++)
            {
                cout << "[ ";
                for (auto& coeff : sk_j.get(r, c).get())
                {
                    cout << coeff << ' ';
                    break;
                }
                cout << "] ";
            }
            cout << "]\n";
        }
    }

    cout << "\n\npublic key" << '\n';
    for (int j = context.depth(); j >= 0; j--)
    {
        auto pk_j = pk.data(j);

        cout << "\nl = " << j << ", dimension = " << pk_j.row_size() << " x " << pk_j.col_size() << '\n';

        for (int r = 0; r < pk_j.row_size(); r++)
        {
            cout << "[ ";
            for (int c = 0; c < pk_j.col_size(); c++)
            {    
                cout << "[ ";
                for (auto& coeff : pk_j.get(r, c).get())
                {
                    cout << coeff << ' ';
                }
                cout << "] ";
            }
            cout << "]\n";
        }
    }*/

    // encoder test
    cout << "\n\n========================== BGV Encoder ==========================\n";

    fheprac::Encoder encoder(context);

    vector<int64_t> v1(context.poly_modulus_degree());
    vector<int64_t> v2(context.poly_modulus_degree());
    vector<int64_t> v3(context.poly_modulus_degree());

    int64_t value = context.plain_modulus_value() / 2;
    for (int i = 0; i < context.poly_modulus_degree() ; i++)
    {
        v1[i] = i + 8000;
        v3[i] = -v1[i];
    }
    v2[0] = 2;

    fheprac::Plaintext p1;
    fheprac::Plaintext p2;
    fheprac::Plaintext p3;
    vector<int64_t> v1_de;
    vector<int64_t> v2_de;
    vector<int64_t> v3_de;

    encoder.encode(v1, p1);
    encoder.encode(v2, p2);
    encoder.encode(v3, p3);

    encoder.decode(p1, v1_de);
    encoder.decode(p2, v2_de);
    encoder.decode(p3, v3_de);

    cout << "\nEncoder: \n";

    bool is_correct_decode = true;
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        if (v1[i] != v1_de[i] || v2[i] != v2_de[i] || v3[i] != v3_de[i])
        {
            is_correct_decode = false;
            break;
        }
    }

    cout << (is_correct_decode ? "\ncorrect answer\n" : "\nwrong answer\n");

    // encryptor test
    cout << "\n\n========================== BGV Encryptor ==========================\n";

    fheprac::Encryptor encryptor(context, pk);
    fheprac::Ciphertext c1;
    fheprac::Ciphertext c2;
    fheprac::Ciphertext c3;

    encryptor.encrypt(p1, c1);
    encryptor.encrypt(p2, c2);
    encryptor.encrypt(p3, c3);

    cout << "\nEncryptor: \n";
    /*for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << c1.data().get(0, 0, i) << ' ' << c1.data().get(1, 0, i) << '\n';
    }*/

    // decryptor test
    cout << "\n\n========================== BGV Decryptor ==========================\n";

    fheprac::Decryptor decryptor(context, sk);
    fheprac::Plaintext p1_dc;
    fheprac::Plaintext p2_dc;
    fheprac::Plaintext p3_dc;

    vector<int64_t> v1_dc;
    vector<int64_t> v2_dc;
    vector<int64_t> v3_dc;

    decryptor.decrypt(c1, p1_dc);
    decryptor.decrypt(c2, p2_dc);
    decryptor.decrypt(c3, p3_dc);

    encoder.decode(p1_dc, v1_dc);
    encoder.decode(p2_dc, v2_dc);
    encoder.decode(p3_dc, v3_dc);

    cout << "\nDecryptor: \n";

    bool is_correct_decrypt = true;
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        if (v1[i] != v1_dc[i] || v2[i] != v2_dc[i] || v3[i] != v3_dc[i])
        {
            is_correct_decrypt = false;
            break;
        }
    }

    cout << (is_correct_decrypt ? "\ncorrect answer\n" : "\nwrong answer\n");

    // modulus switching test
    //cout << "\n\n========================== BGV Modulus Switching ==========================\n";

    fheprac::Evaluator evaluator(context);
 /*   fheprac::Ciphertext c1_ms = c1;
    fheprac::Ciphertext c2_ms = c2;
    fheprac::Plaintext p1_ms;
    fheprac::Plaintext p2_ms;
    vector<int64_t> v1_ms;
    vector<int64_t> v2_ms;

    for (int j = 0; j < context.depth(); j++)
    {
        cout << "\nModulus Switching: \n";
        cout << "l = " << context.depth() - j << " -> " << context.depth() - j - 1 << '\n';
        cout << "q: " << c1_ms.params().q() << " -> ";

        evaluator.mod_switch(c1_ms, c1_ms);
        evaluator.mod_switch(c2_ms, c2_ms);

        cout << c1_ms.params().q() << '\n';

        decryptor.decrypt(c1_ms, p1_ms);
        decryptor.decrypt(c2_ms, p2_ms);

        encoder.decode(p1_ms, v1_ms);
        encoder.decode(p2_ms, v2_ms);

        bool is_correct_mod_switch = true;
        for (int i = 0; i < context.poly_modulus_degree(); i++)
        {
            cout << v1_ms[i] << ' ' << v2_ms[i] << '\n';
            if (v1[i] != v1_ms[i] || v2[i] != v2_ms[i])
            {
                is_correct_mod_switch = false;
                //break;
            }
        }

        cout << (is_correct_mod_switch ? "\ncorrect answer\n" : "\nwrong answer\n");
    }*/

    // add test
    cout << "\n\n========================== BGV Add, Sub, Multiply ==========================\n";
    fheprac::Ciphertext c4;
    fheprac::Ciphertext c4_ms;
    fheprac::Plaintext p4_dc;
    fheprac::Plaintext p4_ms;
    vector<int64_t> v4_dc;
    vector<int64_t> v4_ms;

    evaluator.add(c1, c1, c4);
    evaluator.add(c4, c3, c4);
    evaluator.multiply(c4, c2, c4);
    //evaluator.multiply(c1, c2, c4);

    decryptor.decrypt(c4, p4_dc);
    encoder.decode(p4_dc, v4_dc);

    evaluator.mod_switch(c4, c4_ms);
    decryptor.decrypt(c4_ms, p4_ms);
    encoder.decode(p4_ms, v4_ms);

    cout << "\nAdd, Sub, Multiply: \n";

    bool is_correct_add = true;
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << v4_dc[i] << ' ' << v4_ms[i] << '\n';

        int64_t ans = v1[i] * 2;
        if (ans > context.plain_modulus_value() / 2) {
            ans -= context.plain_modulus_value();
        }
        if (ans != v4_dc[i])
        {
            is_correct_add = false;
            //break;
        }
    }

    cout << (is_correct_add ? "\ncorrect answer\n" : "\nwrong answer\n");


    cout << "\n\n========================== BGV Relinearize ==========================\n";
    fheprac::Ciphertext c4_rl;
    fheprac::Ciphertext c4_rm;
    fheprac::Plaintext p4_rl;
    fheprac::Plaintext p4_rm;
    vector<int64_t> v4_rl;
    vector<int64_t> v4_rm;

    evaluator.relinearize(c4, rk, c4_rl);
    decryptor.decrypt(c4_rl, p4_rl);
    encoder.decode(p4_rl, v4_rl);

    evaluator.mod_switch(c4_rl, c4_rm);
    decryptor.decrypt(c4_rm, p4_rm);
    encoder.decode(p4_rm, v4_rm);

    cout << "\nRelinearize: \n";

    bool is_correct_relinearize = true;
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << v4_rl[i] << ' ' << v4_rm[i] << '\n';

        int64_t ans = v1[i] * 2;
        if (ans > context.plain_modulus_value() / 2) {
            ans -= context.plain_modulus_value();
        }
        if (ans != v4_rl[i])
        {
            is_correct_relinearize = false;
            //break;
        }
    }

    cout << (is_correct_relinearize ? "\ncorrect answer\n" : "\nwrong answer\n");

   // matrix_test();
    //vector_test();
   // polynomial_test();

    /*she::SHE& bgv1 = she::SHEBuilder()
        .sec_level(she::sec_level_t::tc128)
        .mul_mode(she::mul_mode_t::convolution)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_integer_scheme(she::int_scheme_t::bgv, static_cast<size_t>(pow(2, 14)), 50, { 60, 60, 60 });

    she::SHE& bgv2 = she::SHEBuilder()
        .sec_level(she::sec_level_t::tc128)
        .mul_mode(she::mul_mode_t::convolution)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_integer_scheme(she::int_scheme_t::bgv, static_cast<size_t>(pow(2, 14)), 20, { 40, 40, 40 });


    vector<int64_t> v(10, 5);

    auto v_e1 = bgv1.encrypt(bgv1.encode(v));
    auto v_e2 = bgv1.encrypt(bgv2.encode(v));

    cout << v_e1.size() << '\n';
    cout << v_e2.size() << '\n';*/

    /*long long max_count = 0;
    long long max_target = -1;

    for (long long target = 2; target <= 1000000000000LL; ++target) { // 테스트용 작은 범위
        auto [count, pairs] = count_pairs(target);
        if (count > max_count) {
            max_count = count;
            max_target = target;
            cout << "Target: " << target << ", 쌍의 개수: " << count << endl;
        }
    }

    cout << "최대 쌍의 개수를 가진 Target: " << max_target << ", 쌍의 개수: " << max_count << endl;*/


    /*int32_t text_len = pow(2, 15);
    vector<tuple<int32_t, int32_t, int32_t>> test_set;

    for (int32_t& int_set : vector<int32_t>{ 32 })
    {
        for (size_t pattern_len = 10; pattern_len <= 10000; pattern_len += 10)
        {
            test_set.push_back({ text_len , pattern_len, int_set });
        }
    }

    for (auto& [text_size, pattern_size, unique_int_cnt] : test_set)
    {
        integer_matching(text_size, pattern_size, unique_int_cnt);
    }*/

    return 0;
}



/*void bgv_test_1() {
    SHE& bgv = SHEBuilder()
        .sec_level(sec_level_t::tc128)
        .mul_mode(mul_mode_t::convolution)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_integer_scheme(she::int_scheme_t::bgv, static_cast<size_t>(pow(2, 14)), 20);

    cout << bgv.plain_modulus_prime() / 2 << '\n';

    vector<int64_t> v1 = { 1,2,3,4,5,6,7,8,9,10 };
    vector<int64_t> v2 = { 2 };

    auto v1_p = bgv.encode(v1);
    auto v2_p = bgv.encode(v2);

    auto v1_c = bgv.encrypt(v1_p);
    auto v2_c = bgv.encrypt(v2_p);

    Ciphertext temp_c = v1_c;

    for (int i = 0; i < 8; i++) {
        temp_c = bgv.multiply(temp_c, v2_c);
        auto res_p = bgv.decrypt(temp_c);
        auto res = bgv.decode<int64_t>(res_p);
        print_vector(res, 10);
    }

    temp_c = bgv.add(temp_c, v1_p);
    auto res_p = bgv.decrypt(temp_c);
    auto res = bgv.decode<int64_t>(res_p);
    print_vector(res, 10);
}

void bgv_test_2() {
    SHE& bgv = SHEBuilder()
        .sec_level(sec_level_t::tc128)
        .mul_mode(mul_mode_t::element_wise)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_integer_scheme(she::int_scheme_t::bgv, static_cast<size_t>(pow(2, 14)), 30);

    cout << bgv.plain_modulus_prime() / 2 << '\n';

    vector<int64_t> v1 = { 1,2,3,4,5,6,7,8,9,10 };
    vector<int64_t> v2 = { 2 };

    auto v1_p = bgv.encode(v1);
    auto v2_p = bgv.encode(v2);

    auto v1_c = bgv.encrypt(v1_p);
    auto v2_c = bgv.encrypt(v2_p);

    Ciphertext temp_c = v1_c;

    for (int i = 0; i < 4; i++) {
        temp_c = bgv.multiply(temp_c, v2_p);
        auto res_p = bgv.decrypt(temp_c);
        auto res = bgv.decode<int64_t>(res_p);
        print_vector(res, 10);
    }

    temp_c = bgv.add(temp_c, v1_p);
    auto res_p = bgv.decrypt(temp_c);
    auto res = bgv.decode<int64_t>(res_p);
    print_vector(res, 10);
}

void bgv_test_3() {
    SHE& bgv = SHEBuilder()
        .sec_level(sec_level_t::tc128)
        .mul_mode(mul_mode_t::convolution)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_real_complex_scheme(she::real_complex_scheme_t::ckks, static_cast<size_t>(pow(2, 14)), pow(2, 40));

    vector<complex<double_t>> v1 = { {1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{8,8},{9,9},{10,10} };
    vector<complex<double_t>> v2 = { {2,0} };

    auto v1_p = bgv.encode(v1);
    auto v2_p = bgv.encode(v2);

    auto v1_c = bgv.encrypt(v1_p);
    auto v2_c = bgv.encrypt(v2_p);

    Ciphertext temp_c = v1_c;

    for (int i = 0; i < 7; i++) {
        temp_c = bgv.multiply(temp_c, v2_c);
        auto res_p = bgv.decrypt(temp_c);
        auto res = bgv.decode<complex<double_t>>(res_p);
        print_vector(res, 10, 3);
    }

    temp_c = bgv.add(temp_c, v1_p);
    auto res_p = bgv.decrypt(temp_c);
    auto res = bgv.decode<complex<double_t>>(res_p);
    print_vector(res, 10, 3);
}

void bgv_test_4() {
    SHE& bgv = SHEBuilder()
        .sec_level(sec_level_t::tc128)
        .mul_mode(mul_mode_t::element_wise)
        .secret_key(true)
        .public_key(true)
        .relin_keys(true)
        .galois_keys(false)
        .build_real_complex_scheme(she::real_complex_scheme_t::ckks, static_cast<size_t>(pow(2, 14)), pow(2, 60), { 60, 60, 60, 60 ,60 });

    vector<double_t> v1 = { 1,2,3,4,5,6,7,8,9,10 };

    vector<double_t> v2 = { 1 };

    auto v1_p = bgv.encode(v1);
    auto v2_p = bgv.encode(v2);

    auto v1_c = bgv.encrypt(v1_p);
    auto v2_c = bgv.encrypt(v2_p);

    Ciphertext temp_c = v1_c;

    auto temp_p = bgv.decrypt(temp_c);
    auto temp = bgv.decode<double_t>(temp_p);
    print_vector(temp, 10, 18);

    for (int i = 0; i < 4; i++) {
        temp_c = bgv.multiply(temp_c, v2_p);
        auto res_p = bgv.decrypt(temp_c);
        auto res = bgv.decode<double_t>(res_p);
        print_vector(res, 10, 18);
    }

    temp_c = bgv.add(temp_c, v1_p);
    auto res_p = bgv.decrypt(temp_c);
    auto res = bgv.decode<double_t>(res_p);
    print_vector(res, 10, 18);
}

vector<complex<double>> calculate_powers_of_root(size_t n) {
    const double PI = 3.1415926535897932384626433832795028842;
    if (n == 0) {
        throw invalid_argument("n must be greater than 0");
    }

    vector<complex<double>> powers(n + 1);

    for (size_t k = 0; k <= n; ++k) {
        double angle = 2.0 * PI * k / static_cast<double>(n);
        powers[k] = polar(1.0, angle);
    }

    return powers;
}*/