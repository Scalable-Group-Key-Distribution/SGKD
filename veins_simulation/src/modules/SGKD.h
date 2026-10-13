#ifndef SGKD_H_
#define SGKD_H_

#include <iostream>
#include <vector>
#include<chrono>
extern "C" {
#include <relic/relic.h>
}

namespace gkd_vanet {

// Data structures for system parameters and key materials
struct SystemParameters {
    ep_t g1;       // Generator g in G1
    ep2_t g2;
    ep2_t A;       // Group public parameter A in G1
};

struct SystemSecrets {
    bn_t sk;      // Global secret sk in Z_p
    ep_t h;       // Secret h = g^t in G1
};

struct VehicleSecrets {
    bn_t x_i;     // Vehicle secret identifier x_i in Z_p
    ep_t w_i1;    // w_i,1 = h^(x_i + sk) * g^(alpha_i)
    ep2_t w_i2;    // w_i,2 = A^(1 / (x_i + sk))
    ep_t w_i3;    // w_i,3 = g^(alpha_i / (x_i + sk))
};

class SGKDEngine {
public:
    static bool init();
    static void clean();

    static void Setup(SystemParameters &PP, SystemSecrets &Secrets);
    static void AddMember(const SystemParameters &PP, const SystemSecrets &Secrets, VehicleSecrets &v_sec);
    static void DeriveKey(const ep_t w_i1, const ep2_t w_i2, const ep_t w_i3, const ep2_t A, gt_t K);
    static void RevokeMember(SystemParameters &PP, const SystemSecrets &Secrets, const bn_t x_r);
    static void UpdateGroupKey(SystemParameters &PP, const SystemSecrets &Secrets, bn_t x_dummy);
    static void UpdateSecrets(const bn_t x_r, const ep2_t A_hat, VehicleSecrets &v_sec, gt_t K_hat);
    static void benchmarkPairing(double&,int);
};

} // namespace sgkd

#endif /* SGKD_H_ */
