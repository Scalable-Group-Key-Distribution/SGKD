#include "SGKD.h"

namespace gkd_vanet {

bool SGKDEngine::init() {
    if (core_init() != RLC_OK) return false;
    if (pc_param_set_any() != RLC_OK) return false;
    return true;
}

void SGKDEngine::clean() {
    core_clean();
}

void SGKDEngine::Setup(SystemParameters &PP, SystemSecrets &Secrets) {
    bn_t u, t, order;
    bn_null(u); bn_null(t); bn_null(order);
    bn_new(u);  bn_new(t);  bn_new(order);

    g1_get_ord(order);

    bn_rand_mod(u, order);
    bn_rand_mod(t, order);
    bn_rand_mod(Secrets.sk, order);

    g1_get_gen(PP.g1);
    g2_get_gen(PP.g2);
    g1_mul(Secrets.h, PP.g1, t);
    g2_mul(PP.A, PP.g2, u);

    bn_free(u); bn_free(t); bn_free(order);
}

void SGKDEngine::AddMember(const SystemParameters &PP, const SystemSecrets &Secrets, VehicleSecrets &v_sec) {
    bn_t order, alpha_i, temp_exp, inv_exp, alpha_inv;
    bn_null(order); bn_null(alpha_i); bn_null(temp_exp); bn_null(inv_exp); bn_null(alpha_inv);
    bn_new(order);  bn_new(alpha_i);  bn_new(temp_exp);  bn_new(inv_exp);  bn_new(alpha_inv);

    g1_get_ord(order);

    bn_rand_mod(v_sec.x_i, order);
    bn_rand_mod(alpha_i, order);

    //xi+sk
    bn_add(temp_exp, v_sec.x_i, Secrets.sk);
    bn_mod(temp_exp, temp_exp, order);

    //1/(xi+sk)
    //bn_gcd_ext(inv_exp, temp_exp, NULL, temp_exp, order);
    //bn_mod(inv_exp, inv_exp, order);
    bn_mod_inv(inv_exp, temp_exp,order);

    ep_t term1, term2; //term1=g1^{(xi+sk)} while term2=g1^alphai
    ep_null(term1); ep_null(term2); ep_new(term1); ep_new(term2);

    g1_mul(term1, Secrets.h, temp_exp);
    g1_mul(term2, PP.g1, alpha_i);
    // w1=terms*term2
    g1_add(v_sec.w_i1, term1, term2);
    // w2=A^{1/(xi+sk)}
    g2_mul(v_sec.w_i2, PP.A, inv_exp);

    // w3=g1^{alphai/(xi+sk)}
    bn_mul(alpha_inv, alpha_i, inv_exp);
    bn_mod(alpha_inv, alpha_inv, order);
    g1_mul(v_sec.w_i3, PP.g1, alpha_inv);

    ep_free(term1); ep_free(term2);
    bn_free(order); bn_free(alpha_i); bn_free(temp_exp); bn_free(inv_exp); bn_free(alpha_inv);
}

void SGKDEngine::DeriveKey(const ep_t w_i1, const ep2_t w_i2, const ep_t w_i3, const ep2_t A, gt_t K) {
    gt_t num, den;
    gt_null(num); gt_null(den);
    gt_new(num);  gt_new(den);

    pc_map(num, w_i1, w_i2);
    pc_map(den, w_i3, A);

    gt_inv(den, den);
    gt_mul(K, num, den);

    gt_free(num); gt_free(den);
}

void SGKDEngine::RevokeMember(SystemParameters &PP, const SystemSecrets &Secrets, const bn_t x_r) {
    bn_t order, sum_exp, inv_exp;
    bn_null(order); bn_null(sum_exp); bn_null(inv_exp);
    bn_new(order);  bn_new(sum_exp);  bn_new(inv_exp);

    g1_get_ord(order);

    bn_add(sum_exp, x_r, Secrets.sk);
    bn_mod(sum_exp, sum_exp, order);

//    bn_gcd_ext(inv_exp, sum_exp, NULL, sum_exp, order);
//    bn_mod(inv_exp, inv_exp, order);
    bn_mod_inv(inv_exp, sum_exp, order);

    g2_mul(PP.A, PP.A, inv_exp);

    bn_free(order); bn_free(sum_exp); bn_free(inv_exp);
}

void SGKDEngine::UpdateGroupKey(SystemParameters &PP, const SystemSecrets &Secrets, bn_t x_dummy) {
    bn_t order;
    bn_null(order); bn_new(order);
    g1_get_ord(order);

    bn_rand_mod(x_dummy, order);
    RevokeMember(PP, Secrets, x_dummy);

    bn_free(order);
}

void SGKDEngine::UpdateSecrets(const bn_t x_r, const ep2_t A_hat, VehicleSecrets &v_sec, gt_t K_hat) {
    bn_t order, diff, exp_e;
    bn_null(order); bn_null(diff); bn_null(exp_e);
    bn_new(order);  bn_new(diff);  bn_new(exp_e);

    g1_get_ord(order);

    bn_sub(diff, v_sec.x_i, x_r);
    bn_mod(diff, diff, order);

//    bn_gcd_ext(exp_e, diff, NULL, diff, order);
//    bn_mod(exp_e, exp_e, order);
    bn_mod_inv(exp_e, diff, order);

    ep2_t w_i2_inv, term;
    ep2_null(w_i2_inv); ep2_null(term);
    ep2_new(w_i2_inv);  ep2_new(term);

    g2_neg(w_i2_inv, v_sec.w_i2);
    g2_add(term, A_hat, w_i2_inv);
    g2_mul(v_sec.w_i2, term, exp_e);

    DeriveKey(v_sec.w_i1, v_sec.w_i2, v_sec.w_i3, A_hat, K_hat);

    ep2_free(w_i2_inv); ep2_free(term);
    bn_free(order); bn_free(diff); bn_free(exp_e);
}
void SGKDEngine::benchmarkPairing(double& avgMs, int iterations = 1000) {
    if (iterations <= 0) {
        std::cerr << "Error: Iterations must be a positive integer.\n";
        return;
    }
    // 1. Initialize RELIC variables
    ep_t g1_point;
    ep2_t g2_point;
    gt_t gt_result;

    ep_null(g1_point);   ep_new(g1_point);
    ep2_null(g2_point);  ep2_new(g2_point);
    gt_null(gt_result);  gt_new(gt_result);

    // 2. Generate random test inputs in G1 and G2
    ep_rand(g1_point);
    ep2_rand(g2_point);

    // Warm-up iteration (to prime instruction/data caches)
//    pp_map_oate(gt_result, g1_point, g2_point);
    pc_map(gt_result, g1_point, g2_point);
    // 3. Execution Benchmark Loop
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // Compute bilinear pairing e(P, Q) -> GT
//        pp_map_oate(gt_result, g1_point, g2_point);
        pc_map(gt_result, g1_point, g2_point);
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    // 4. Calculate Timing Metrics
    std::chrono::duration<double, std::micro> totalDurationUs = endTime - startTime;
    double totalMs = totalDurationUs.count() / 1000.0;
    avgMs = totalMs / iterations;
    double avgUs = totalDurationUs.count() / iterations;
    double opsPerSec = (iterations / totalMs) * 1000.0;

    // 5. Display Benchmark Results
//    std::cout << std::fixed << std::setprecision(4);
//    std::cout << "Total Execution Time : " << totalMs << " ms\n";
//    std::cout << "Average Pairing Latency: " << avgMs << " ms (" << avgUs << " us)\n";
//    std::cout << "Throughput            : " << opsPerSec << " pairings/sec\n";
//    std::cout << "========================================================\n";

    // 6. Memory Cleanup
    ep_free(g1_point);
    ep2_free(g2_point);
    gt_free(gt_result);
}
} // namespace gkd_vanet
