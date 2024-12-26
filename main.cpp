#include "examples.h"
#include "modules/io/teestream.h"
#include "modules/fhe/fhebuilder.h"
#include "modules/fhe/fhe.h"
#include "modules/random/randomgenerator.h"
#include "modules/simulator/patternmatch.h"
#include "modules/fhe-prac/bgv/context.h"
#include "modules/fhe-prac/bgv/keygenerator.h"
#include "modules/fhe-prac/bgv/secretkey.h"
#include "modules/fhe-prac/bgv/encoder.h"
#include "modules/fhe-prac/bgv/plaintext.h"
#include "modules/fhe-prac/bgv/ciphertext.h"
#include "modules/fhe-prac/bgv/encryptor.h"
#include "modules/fhe-prac/bgv/decryptor.h"
#include "modules/fhe-prac/bgv/evaluator.h"
#include "modules/fhe-prac/bgv/util/polynomial.h"
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


int main()
{
    cout << (1148844456385434516 * 1148844456385434514) % 1148844456385434517 << '\n';

    cout << safe_modular_multiplication(1148844456385434516, 1148844456385434514, 1148844456385434517) << '\n';

    // context text
    cout << "\n========================== Context ==========================\n";

    fheprac::Context context(4, 10, 3);

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
    cout << "\n========================== KeyGenerator ==========================\n";

    fheprac::KeyGenerator key_gen(context);
    fheprac::SecretKey sk;
    fheprac::PublicKey pk;

    sk = key_gen.secret_key();
    key_gen.create_public_key(pk);

    cout << "\nsecret key" << '\n';
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
    }

    // encoder test
    cout << "\n========================== BGV Encoder ==========================\n";

    fheprac::Encoder encoder(context);

    vector<int64_t> v1(context.poly_modulus_degree());
    vector<int64_t> v2(context.poly_modulus_degree());

    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        v1[i] = i + 1;
        v2[i] = (i * 10) - 15;
    }

    fheprac::Plaintext p1;
    fheprac::Plaintext p2;

    encoder.encode(v1, p1);
    encoder.encode(v2, p2);

    v1.clear();
    v2.clear();

    encoder.decode(p1, v1);
    encoder.decode(p2, v2);

    cout << "\nencode and decode\n";
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << v1[i] << ' ' << v2[i] << '\n';
    }

    // encryptor test
    cout << "\n========================== BGV Encryptor ==========================\n";

    fheprac::Encryptor encryptor(context, pk);

    fheprac::Ciphertext c1;
    fheprac::Ciphertext c2;

    encryptor.encrypt(p1, c1);
    encryptor.encrypt(p2, c2);

    cout << "\nencrypt\n";
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << c1.data().get(0, 0, i) << ' ' << c1.data().get(1, 0, i) << '\n';
    }

    // decryptor test
    cout << "\n========================== BGV Decryptor ==========================\n";

    fheprac::Decryptor decryptor(context, sk);

    decryptor.decrypt(c1, p1);
    decryptor.decrypt(c2, p2);

    encoder.decode(p1, v1);
    encoder.decode(p2, v2);

    cout << "\ndecrypt" << '\n';
    for (int i = 0; i < context.poly_modulus_degree(); i++)
    {
        cout << v1[i] << ' ' << v2[i] << '\n';
    }

    // modulus switching test
    cout << "\n========================== BGV Modulus Switching ==========================\n";

    fheprac::Evaluator evaluator(context);
    fheprac::Ciphertext c1_ms = c1;
    fheprac::Ciphertext c2_ms = c2;

    for (int j = 0; j < context.depth(); j++)
    {
        cout << '\n';
        cout << "l = " << context.depth() - j << " -> " << context.depth() - j - 1 << '\n';
        cout << "q: " << c1_ms.param().q() << " -> ";

        evaluator.mod_switch(c1_ms, c1_ms);
        evaluator.mod_switch(c2_ms, c2_ms);

        cout << c1_ms.param().q() << '\n';

        decryptor.decrypt(c1_ms, p1);
        decryptor.decrypt(c2_ms, p2);

        encoder.decode(p1, v1);
        encoder.decode(p2, v2);

        cout << '\n';
        for (int i = 0; i < context.poly_modulus_degree(); i++)
        {
            cout << v1[i] << ' ' << v2[i] << '\n';
        }
    }

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