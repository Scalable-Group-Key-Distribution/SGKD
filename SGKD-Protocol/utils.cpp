#include <utility>
#include <string>
#include <iostream>
#include <relic/relic_pc.h>
#include <sys/socket.h>
#include <vector>
using namespace std;
#define BUF_SIZE 2048
template <typename F>
pair<double, double> benchmark_stats(F func, int inner_loop = 1000, int outer_loop = 100) {
    vector<double> times;
    times.reserve(outer_loop);

    for (int i = 0; i < outer_loop; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < inner_loop; ++j) func();
        auto end = std::chrono::high_resolution_clock::now();
        double total = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        times.push_back(total / inner_loop);
    }

    double sum = 0.0;
    for (double t : times) sum += t;
    double mean = sum / outer_loop;

    double sq_sum = 0.0;
    for (double t : times) sq_sum += (t - mean) * (t - mean);
    double stddev = sqrt(sq_sum / outer_loop);

    return {mean, stddev};
}
void handle_error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
    exit(EXIT_FAILURE);
}
void send_element(int sock, const g1_t& el) {
    uint8_t buffer[BUF_SIZE];
    int len = g1_size_bin(el, 1); // compressed
    std::cout<<"g1 len:"<<len<<std::endl;
    g1_write_bin(buffer, len, el, 1);
    send(sock, &len, sizeof(len), 0); 
    send(sock, buffer, len, 0);
}

void send_element(int sock, const g2_t& el) {
    uint8_t buffer[BUF_SIZE];
    int len = g2_size_bin(el, 1); // compressed
    std::cout<<"g2 len:"<<len<<std::endl;
    g2_write_bin(buffer, len, el, 1);
    send(sock, &len, sizeof(len), 0); 
    send(sock, buffer, len, 0);
}
void send_bn(int sock, const bn_t& n) {
    uint8_t buffer[BUF_SIZE];
    int len = bn_size_bin(n);
    std::cout<<"bn len:"<<len<<std::endl;
    bn_write_bin(buffer, len, n);
    send(sock, &len, sizeof(len), 0); 
    send(sock, buffer, len, 0);
}
std::vector<uint8_t> serialize_element(g1_t elem) {
    int len = g1_size_bin(elem, 1);
    std::vector<uint8_t> buf(len);
    g1_write_bin(buf.data(), len, elem, 1);
    return buf;
}
std::vector<uint8_t> serialize_element(g2_t elem) {
    int len = g2_size_bin(elem, 1);
    std::vector<uint8_t> buf(len);
    g2_write_bin(buf.data(), len, elem, 1);
    return buf;
}

std::vector<uint8_t> serialize_element(bn_t elem) {
    int len = bn_size_bin(elem);
    std::vector<uint8_t> buf(len);
    bn_write_bin(buf.data(), len, elem);
    return buf;
}

void deserialize_element(g1_t elem, const uint8_t* data, int len) {
    g1_read_bin(elem, data, len);
}

void deserialize_element(g2_t elem, const uint8_t* data, int len) {
    g2_read_bin(elem, data, len);
}


void deserialize_element(bn_t elem, const uint8_t* data, int len) {
    bn_read_bin(elem, data, len);
}
