#include <iostream>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <relic/relic.h>
#include <openssl/sha.h>
#include<chrono>
#include<utility>
#include <cmath>
#include <numeric>
#include"utils.cpp"
using namespace std;
#define TA_IP "127.0.0.1"
#define TA_PORT 9876
#define TA_ACK_PORT 9998
#define BROADCAST_PORT 9999
#define BUF_SIZE 4096

//compile it using: g++ vehicle.cpp -o vehicle   -I/usr/local/relic/include   -L/usr/local/relic/lib -lrelic_s   -lssl -lcrypto   -std=c++17
void derive_key(g1_t w1, g2_t w2) {
    gt_t pairing_result;
    gt_null(pairing_result); gt_new(pairing_result);
    pc_map(pairing_result, w1, w2);

    uint8_t buffer[512];
    int len = gt_size_bin(pairing_result, 1);
    gt_write_bin(buffer, len, pairing_result, 1);

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(buffer, len, hash);

    std::cout << "[*] Session Key: ";
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        printf("%02x", hash[i]);
    std::cout << std::endl;
}
void UpdateMemberSecrets(const g1_t& w1, g2_t& w2, const bn_t& x_i, const g2_t& A_new, const bn_t& x_r) {
    bn_t ord, exp;
    bn_new(ord); bn_new(exp);
    ep_curve_get_ord(ord);

    // e = 1 / (x_i - x_r)
    bn_sub(exp, x_i, x_r);
    bn_mod_inv(exp, exp, ord);

    g2_t w2_old_exp, A_exp, w2_new;
    g2_new(w2_old_exp); g2_new(A_exp); g2_new(w2_new);

    // w2_old_exp = w2^e
    g2_mul(w2_old_exp, w2, exp);
    // A_exp = A^e
    g2_mul(A_exp, A_new, exp);

    // w2_new = A^e / w2^e
    g2_neg(w2_old_exp, w2_old_exp);         // Point negation
    g2_add(w2_new, A_exp, w2_old_exp);      // Group addition (A_exp * w2_old_exp^{-1})

    // Update stored w2
    g2_copy(w2, w2_new);

    // Derive new key
    gt_t shared;
    gt_new(shared);
    pc_map(shared, w1, w2);
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256((uint8_t*)shared, sizeof(shared), hash);

    std::cout << "[INFO] New session key derived (SHA256 hash of pairing result)" << std::endl;

    // Cleanup
    bn_free(ord); bn_free(exp);
    g2_free(w2_old_exp); g2_free(A_exp); g2_free(w2_new);
    gt_free(shared);
}

void listen_for_key_update(const g1_t& w1, g2_t& w2, const bn_t& x_i) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP socket creation failed");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("UDP bind failed");
        close(sockfd);
        return;
    }

    std::cout << "[INFO] Listening for key updates on UDP port " << BROADCAST_PORT << std::endl;

    uint8_t buffer[BUF_SIZE];

    while (true) {
        ssize_t len = recv(sockfd, buffer, BUF_SIZE, 0);
        if (len <= 0) continue;

        int offset = 0;

        g2_t A_recv;
        bn_t x_r;
        g2_new(A_recv);
        bn_new(x_r);

        int len_g2 = g2_size_bin(A_recv, 1);
        g2_read_bin(A_recv, buffer + offset, len_g2);
        offset += len_g2;

        int len_xr = len - offset;
        bn_read_bin(x_r, buffer + offset, len_xr);

        std::cout << "[INFO] Key update received: updating member secrets..." << std::endl;
        UpdateMemberSecrets(w1, w2, x_i, A_recv, x_r);

        g2_free(A_recv); bn_free(x_r);
    }

    close(sockfd);
}
//The listen_for_key_update_benchmark function used in measuring the end-to-end latency of vehicle revocation/key update process
void listen_for_key_update_benchmark(const g1_t& w1, g2_t& w2, const bn_t& x_i, const std::string& vehicle_id) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP socket creation failed");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("UDP bind failed");
        close(sockfd);
        return;
    }

    std::cout << "[INFO] Listening for key updates on UDP port " << BROADCAST_PORT << std::endl;

    uint8_t buffer[BUF_SIZE];

    while (true) {
        ssize_t len = recv(sockfd, buffer, BUF_SIZE, 0);
        if (len <= 0) continue;

        int offset = 0;

        g2_t A_recv;
        bn_t x_r;
        g2_new(A_recv);
        bn_new(x_r);

        int len_g2 = g2_size_bin(A_recv, 1);
        g2_read_bin(A_recv, buffer + offset, len_g2);
        offset += len_g2;

        int len_xr = len - offset;
        bn_read_bin(x_r, buffer + offset, len_xr);

        std::cout << "[INFO] Key update received: updating member secrets..." << std::endl;
        UpdateMemberSecrets(w1, w2, x_i, A_recv, x_r);

        // Send ACK back to TA
        // This is not part of the SGKP protocol. We add it here for the end-to-end latency measurment
        int ack_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (ack_sock >= 0) {
            sockaddr_in ta_addr{};
            ta_addr.sin_family = AF_INET;
            ta_addr.sin_port = htons(TA_ACK_PORT);
            ta_addr.sin_addr.s_addr = inet_addr(TA_IP);

            sendto(ack_sock, vehicle_id.c_str(), vehicle_id.length(), 0,
                   (sockaddr*)&ta_addr, sizeof(ta_addr));

            std::cout << "[INFO] Sent ACK to TA: " << vehicle_id << std::endl;

            close(ack_sock);
        } else {
            perror("ACK socket creation failed");
        }

        g2_free(A_recv); 
        bn_free(x_r);
    }

    close(sockfd);
}
void registervehicle()
{
    bn_t x_i;
    g1_t w1;
    g2_t w2;
    bn_null(x_i); g1_null(w1); g2_null(w2);
    bn_new(x_i); g1_new(w1); g2_new(w2);
    bn_t ord;
    bn_new(ord);
    ep_curve_get_ord(ord);
    bn_rand_mod(x_i,ord);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TA_PORT);
    inet_pton(AF_INET, TA_IP, &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        return;
    }

    const char* id = "veh_id_123456";
    send(sock, id, 16, 0);

    uint8_t buf[2048];

    int offset = 0;
    int len_xi = bn_size_bin(x_i);
    int len_g1 = g1_size_bin(w1, 1);
    int len_g2 = g2_size_bin(w2,1);

    int len;
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    uint8_t buffer[1024];
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    bn_read_bin(x_i, buffer, len);
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    g1_read_bin(w1, buffer, len);
    
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    g2_read_bin(w2, buffer, len);

    derive_key(w1, w2);
    close(sock);
    listen_for_key_update_benchmark(w1,w2,x_i,id);

}
//The registrvehicle_benchmark function is used for measuring the end-to-end latency of the vehicle registration process.
void registervehicle_benchmark()
{
    bn_t x_i;
    g1_t w1;
    g2_t w2;
    bn_null(x_i); g1_null(w1); g2_null(w2);
    bn_new(x_i); g1_new(w1); g2_new(w2);
    bn_t ord;
    bn_new(ord);
    ep_curve_get_ord(ord);
    bn_rand_mod(x_i,ord);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TA_PORT);
    inet_pton(AF_INET, TA_IP, &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        return;
    }

    const char* id = "veh_id_123456";
    send(sock, id, 16, 0);

    uint8_t buf[2048];

    int offset = 0;
    int len_xi = bn_size_bin(x_i);
    int len_g1 = g1_size_bin(w1, 1);
    int len_g2 = g2_size_bin(w2,1);

    int len;
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    uint8_t buffer[1024];
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    bn_read_bin(x_i, buffer, len);
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    g1_read_bin(w1, buffer, len);
    
    recv(sock, &len, sizeof(len), MSG_WAITALL);  // Receive length
    recv(sock, buffer, len, MSG_WAITALL);        // Receive data
    g2_read_bin(w2, buffer, len);

    derive_key(w1, w2);
    close(sock);

}

int main() {
    if (core_init() != RLC_OK || pc_param_set_any() != RLC_OK) {
        std::cerr << "RELIC initialization failed." << std::endl;
        return 1;
    }
    cout<<"========================================================"<<endl;
    cout<<"| Select a test option from the following list:        |"<<endl;
    cout<<"| Press 1 fro the registration end-to-end latency      |"<<endl;
    cout<<"| Press 2 for the update end-to-end latency            |"<<endl;
    cout<<"========================================================"<<endl;
    int scenario=0;
    cin>>scenario;
    switch (scenario)
    {
    case 1:
        {   
            auto [reglatency_avg, reglatency_std] = benchmark_stats(registervehicle_benchmark);
            cout << "Registration Total Latency:" << reglatency_avg << " ns (±" << reglatency_std << ")\n";
        }

        break;
    case 2:
        {
            registervehicle();
        }
    break;    
    default:
        break;
    }
    
    
    return 0;
}
