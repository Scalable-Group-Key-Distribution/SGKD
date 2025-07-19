// #include <relic/relic.h>
// #include <iostream>
// #include <chrono>

// using namespace std;
// using namespace std::chrono;

// #define LOOP_COUNT 1000
// // compile it using: g++ relic-pairing-benchmark.cpp -o pairing-bench -lrelic -lgmp
// int main() {
//     // Initialize RELIC core
//     if (core_init() != RLC_OK) {
//         std::cerr << "RELIC core init failed\n";
//         return 1;
//     }

//     // Set pairing-friendly curve (Type 3 pairing)
//     if (pc_param_set_any() != RLC_OK) {
//         std::cerr << "RELIC pairing parameter setup failed\n";
//         core_clean();
//         return 1;
//     }

//     // Declare elements
//     ep_t P, aP;
//     ep2_t Q, bQ;
//     gt_t result;
//     bn_t a, b, order;

//     ep_new(P); ep_new(aP);
//     ep2_new(Q); ep2_new(bQ);
//     gt_new(result);
//     bn_new(a); bn_new(b); bn_new(order);

//     // Setup random elements
//     ep_curve_get_ord(order);
//     ep_rand(P);
//     ep2_rand(Q);
//     bn_rand_mod(a, order);
//     bn_rand_mod(b, order);
//     ep_mul(aP, P, a);
//     ep2_mul(bQ, Q, b);

//     // Benchmark pairing
//     auto start = high_resolution_clock::now();
//     for (int i = 0; i < LOOP_COUNT; ++i) {
//         pc_map(result, aP, bQ);
//     }
//     auto end = high_resolution_clock::now();
//     auto avg_ns = duration_cast<nanoseconds>(end - start).count() / LOOP_COUNT;

//     cout << "Average pairing time over " << LOOP_COUNT << " iterations: " << avg_ns << " ns" << endl;

//     // Clean up
//     ep_free(P); ep_free(aP);
//     ep2_free(Q); ep2_free(bQ);
//     gt_free(result);
//     bn_free(a); bn_free(b); bn_free(order);
//     core_clean();

//     return 0;
// }
// #include <relic/relic.h>
// #include <iostream>
// #include <chrono>
// #include <vector>
// #include <cmath>

// using namespace std;
// using namespace std::chrono;

// #define LOOP_COUNT 1000
// // Compile with: g++ relic-pairing-benchmark.cpp -o pairing-bench -lrelic -lgmp

// int main() {
//     // Initialize RELIC core
//     if (core_init() != RLC_OK) {
//         cerr << "RELIC core init failed\n";
//         return 1;
//     }

//     // Set pairing-friendly curve (Type 3 pairing)
//     if (pc_param_set_any() != RLC_OK) {
//         cerr << "RELIC pairing parameter setup failed\n";
//         core_clean();
//         return 1;
//     }

//     // Declare elements
//     ep_t P, aP;
//     ep2_t Q, bQ;
//     gt_t result;
//     bn_t a, b, order;

//     ep_new(P); ep_new(aP);
//     ep2_new(Q); ep2_new(bQ);
//     gt_new(result);
//     bn_new(a); bn_new(b); bn_new(order);

//     // Setup random elements
//     ep_curve_get_ord(order);
//     ep_rand(P);
//     ep2_rand(Q);
//     bn_rand_mod(a, order);
//     bn_rand_mod(b, order);
//     ep_mul(aP, P, a);
//     ep2_mul(bQ, Q, b);

//     // Warm-up pairing
//     pc_map(result, aP, bQ);

//     // Benchmark pairing with individual timing
//     vector<double> durations_ns;
//     durations_ns.reserve(LOOP_COUNT);

//     for (int i = 0; i < LOOP_COUNT; ++i) {
//         auto start = high_resolution_clock::now();
//         pc_map(result, aP, bQ);
//         auto end = high_resolution_clock::now();
//         double duration = duration_cast<nanoseconds>(end - start).count();
//         durations_ns.push_back(duration);
//     }

//     // Compute average
//     double sum = 0.0;
//     for (double t : durations_ns) {
//         sum += t;
//     }
//     double avg_ns = sum / LOOP_COUNT;

//     // Compute standard deviation
//     double sq_sum = 0.0;
//     for (double t : durations_ns) {
//         sq_sum += (t - avg_ns) * (t - avg_ns);
//     }
//     double std_dev = sqrt(sq_sum / LOOP_COUNT);

//     cout << "Pairing benchmark over " << LOOP_COUNT << " iterations:\n";
//     cout << "Average time: " << avg_ns << " ns\n";
//     cout << "Standard deviation: " << std_dev << " ns\n";

//     // Clean up
//     ep_free(P); ep_free(aP);
//     ep2_free(Q); ep2_free(bQ);
//     gt_free(result);
//     bn_free(a); bn_free(b); bn_free(order);
//     core_clean();

//     return 0;
// }
#include <relic/relic.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>

using namespace std;
using namespace std::chrono;

#define INNER_LOOP 1000
#define OUTER_LOOP 1000
// Compile with: g++ relic-pairing-benchmark.cpp -o pairing-bench -lrelic -lgmp

// Benchmark function measuring average + stddev over batches
template <typename F>
pair<double, double> benchmark_stats(F func) {
    vector<double> samples;
    samples.reserve(OUTER_LOOP);

    for (int i = 0; i < OUTER_LOOP; ++i) {
        auto start = high_resolution_clock::now();
        for (int j = 0; j < INNER_LOOP; ++j) {
            func();
        }
        auto end = high_resolution_clock::now();
        double batch_time = duration_cast<nanoseconds>(end - start).count();
        samples.push_back(batch_time / INNER_LOOP);
    }

    double sum = 0.0;
    for (double t : samples) sum += t;
    double mean = sum / OUTER_LOOP;

    double sq_sum = 0.0;
    for (double t : samples) sq_sum += (t - mean) * (t - mean);
    double stddev = sqrt(sq_sum / OUTER_LOOP);

    return {mean, stddev};
}

int main() {
    // Initialize RELIC core
    if (core_init() != RLC_OK) {
        cerr << "RELIC core init failed\n";
        return 1;
    }

    // Set pairing-friendly curve (Type 3 pairing)
    if (pc_param_set_any() != RLC_OK) {
        cerr << "RELIC pairing parameter setup failed\n";
        core_clean();
        return 1;
    }

    // Declare elements
    ep_t P, aP;
    ep2_t Q, bQ;
    gt_t result;
    bn_t a, b, order;

    ep_new(P); ep_new(aP);
    ep2_new(Q); ep2_new(bQ);
    gt_new(result);
    bn_new(a); bn_new(b); bn_new(order);

    // Setup random elements
    ep_curve_get_ord(order);
    ep_rand(P);
    ep2_rand(Q);
    bn_rand_mod(a, order);
    bn_rand_mod(b, order);
    ep_mul(aP, P, a);
    ep2_mul(bQ, Q, b);

    // Warm-up pairing
    pc_map(result, aP, bQ);

    // Benchmark pairing using batch timing
    auto [avg_ns, std_dev] = benchmark_stats([&]() {
        pc_map(result, aP, bQ);
    });

    cout << "RELIC pairing benchmark (" << OUTER_LOOP << " batches of " << INNER_LOOP << "):\n";
    cout << "Average time: " << avg_ns << " ns\n";
    cout << "Standard deviation: " << std_dev << " ns\n";

    // Clean up
    ep_free(P); ep_free(aP);
    ep2_free(Q); ep2_free(bQ);
    gt_free(result);
    bn_free(a); bn_free(b); bn_free(order);
    core_clean();

    return 0;
}
