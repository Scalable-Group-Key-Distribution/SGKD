#include <iostream>
#include <vector>
#include <cstring>
#include <relic/relic.h>
#include <relic/relic_core.h>
#include <relic/relic_ep.h>
#include <relic/relic_pp.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<chrono>
#include<utility>
#include <cmath>
#include <numeric>
#include"utils.cpp"
using namespace std;

#define PORT 9876
#define ID_LEN 16
#define BUF_SIZE 2048
#define BROADCAST_PORT 9999
#define BROADCAST_IP "255.255.255.255"
#define ACK_PORT 9998
//compile and build it using:g++ ta.cpp -o ta   -I/usr/local/relic/include   -L/usr/local/relic/lib -lrelic_s   -lssl -lcrypto   -std=c++17
// Global public parameters
g1_t g1, h;
g2_t g2, A;
bn_t sk;
bn_t xr;

void Setup() {
    if (core_init() != RLC_OK) handle_error("RELIC core init failed");
    if (pc_param_set_any() != RLC_OK) handle_error("Pairing params setup failed");

    g1_null(g1); g1_null(h); g2_null(g2);g2_null(A);
    bn_null(sk);

    g1_new(g1); g1_new(h); g2_new(g2);g2_new(A);
    bn_new(sk);

    g1_rand(g1);
    g1_rand(h);
    g2_rand(g2);

    bn_t u, ord;
    bn_new(u);
    bn_new(ord);
    ep_curve_get_ord(ord);     // Get group order p
    bn_rand_mod(u, ord);       // u ∈ Z_p
    bn_rand_mod(sk, ord);

    // A = g^u
    g2_mul(A, g2, u);
    bn_new(xr);
    bn_free(u);
}

void broadcast_key_update(const g2_t& A, const bn_t& x_r) {
    // 1. Prepare serialized data
    uint8_t buffer[4096];
    int offset = 0;

    // Serialize A
    int len_A = g2_size_bin(A, 1);
    g2_write_bin(buffer + offset, len_A, A, 1);
    offset += len_A;

    // Serialize x_r
    int len_xr = bn_size_bin(x_r);
    bn_write_bin(buffer + offset, len_xr, x_r);
    offset += len_xr;

    // 2. Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket creation failed");
        return;
    }

    // 3. Enable broadcast
    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt failed");
        close(sock);
        return;
    }

    // 4. Setup destination address
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(BROADCAST_PORT);
    dest.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    // 5. Send the data
    ssize_t sent = sendto(sock, buffer, offset, 0, (sockaddr*)&dest, sizeof(dest));
    if (sent < 0) {
        perror("Broadcast send failed");
    } else {
        std::cout << "[INFO] Key update broadcasted (" << sent << " bytes)" << std::endl;
    }

    close(sock);
}
void receive_vehicle_acks() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("ACK socket creation failed");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ACK_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("ACK bind failed");
        close(sock);
        return;
    }

    char buffer[256];
    // while (true) {
    ssize_t len = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        std::cout << "[ACK RECEIVED] Vehicle ID: " << buffer << std::endl;
    }
    // }

    close(sock);
}

void AddMember(int sock) {
    // 1. Receive 16-byte ID
    char id[ID_LEN + 1] = {0};
    recv(sock, id, ID_LEN, 0);
    std::cout << "Registering vehicle with ID: " << id << std::endl;
    std::cout<<"test1"<<std::endl;

    // 2. Generate member secret xi
    bn_t xi, temp, inv;
    bn_t ord;
    bn_new(ord);
    ep_curve_get_ord(ord);
    bn_new(xi); bn_new(temp); bn_new(inv);
    do{
    bn_rand_mod(xi, ord);
    }while (bn_is_zero(xi));
    
    
    std::cout<<"test2"<<std::endl;
    g1_t w1;
    g2_t w2;
    g1_new(w1); g2_new(w2);
    
    // temp = xi + sk
    bn_add(temp, xi, sk);
    std::cout<<"test3"<<std::endl;
    // w1 = h^{xi + sk}
    g1_mul(w1, h, temp);
    std::cout<<"test4"<<std::endl;
    // w2 = A^{1/(xi + sk)}
    // bn_gcd_ext(inv, NULL, NULL, temp, ord);
    bn_mod_inv(inv, temp, ord);
    std::cout<<"test5"<<std::endl;
    g2_mul(w2, A, inv);
    std::cout<<"test6"<<std::endl;
    // 3. Send xi, w1, w2 to the vehicle
    send_bn(sock, xi);
    std::cout<<"test7"<<std::endl;
    send_element(sock, w1);
    std::cout<<"test8"<<std::endl;
    send_element(sock, w2);
    std::cout<<"test9"<<std::endl;
    bn_copy(xr,xi);
    bn_free(xi); bn_free(temp); bn_free(inv);
    g1_free(w1); g1_free(w2);
}

void RevokeMember(const bn_t& x_r) {
    bn_t denom, inv, ord;
    bn_new(denom); bn_new(inv); bn_new(ord);
    ep_curve_get_ord(ord);

    bn_add(denom, x_r, sk);
    bn_mod_inv(inv, denom, ord);

    g2_mul(A, A, inv); // A = A^{1/(x_r + sk)}

    // Broadcast A and x_r to all vehicles
    broadcast_key_update(A, x_r);

    bn_free(denom); bn_free(inv); bn_free(ord);
}
void update()
{
    bn_t xi;
    bn_t ord;
    bn_new(ord);
    ep_curve_get_ord(ord);
    bn_new(xi);
    bn_rand_mod(xi, ord);
    RevokeMember(xi);
}
void update_benchmark()
{
    bn_t xi;
    bn_t ord;
    bn_new(ord);
    ep_curve_get_ord(ord);
    bn_new(xi);
    bn_rand_mod(xi, ord);

    RevokeMember(xi);
    receive_vehicle_acks();
}
int setup_listener(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) handle_error("socket creation failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
        handle_error("bind failed");

    if (listen(sockfd, 5) < 0)
        handle_error("listen failed");

    std::cout << "Listening on port " << port << std::endl;
    return sockfd;
}
void showoptionmenu()
{
    std::cout<<"================================================"<<std::endl;
    std::cout<<"| Please select one of the following options: |"<<std::endl;
    std::cout<<"|   1- Accept Vehicle Registration             |"<<std::endl;
    std::cout<<"|   2- Revoke The Last Added Vehicle           |"<<std::endl;
    std::cout<<"|   3- Refresh The Group Key                   |"<<std::endl;
    std::cout<<"|   4- Benchmark Vehicle Registration          |"<<std::endl;
    std::cout<<"|   5- Benchmark The Group Key Update          |"<<std::endl;
    std::cout<<"|   6- Benchmark ACK Latency                   |"<<std::endl;
    std::cout<<"================================================"<<std::endl;
}
int main() {
    Setup();
    int listener = setup_listener(PORT);

    while (true) {
        showoptionmenu();
        int n;
        std::cin>>n;
        switch (n)
        {
        case 1:{
            int sock = accept(listener, nullptr, nullptr);
            if (sock >= 0) {
                AddMember(sock);
                close(sock);
            }
            break;
        }
        case 2:{
            RevokeMember(xr);
            break;
        }
        case 3:{
            update();
            break;
        }
        case 4:{
            cout<<"Please enter the total number of registration:"<<endl;
            int totalregistrations=1;
            cin>>totalregistrations;
            for (size_t i = 0; i < totalregistrations; i++)
            {
               int sock = accept(listener, nullptr, nullptr);
                if (sock >= 0) {
                    AddMember(sock);
                    close(sock);
                }
            }
            
            break;
        }
        case 5:{
            auto [updatelatency_avg, updatelatency_std] = benchmark_stats(update_benchmark);
            cout << "Registration Total Latency:" << updatelatency_avg << " ns (±" << updatelatency_std << ")\n";
            
            break;
        }
        case 6:
        {
            cout<<"Please enter the total number of acks to receive:"<<endl;
            int totalacks=1;
            cin>>totalacks;
            for (size_t i = 0; i < totalacks; i++)
            {
                receive_vehicle_acks();
            }
            
        }
        default:
            break;
        }
        
    }

    core_clean(); // Always clean RELIC before exiting
    return 0;
}
