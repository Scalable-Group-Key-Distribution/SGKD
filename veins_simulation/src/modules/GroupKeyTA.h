#pragma once

#include <omnetpp.h>
#include <set>
#include <string>
#include <sstream>
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "SGKD.h"
namespace gkd_vanet {

class GroupKeyTA : public omnetpp::cSimpleModule {
  protected:
    int currentKeyVersion = 0;
    std::string currentGroupKey;
    omnetpp::cMessage *keyUpdateTimer = nullptr;
    std::set<std::string> registeredVehicles;
    SystemParameters PP;
    SystemSecrets SSecrets;

    long registrationRequests = 0;
    long registrationResponses = 0;
    long keyUpdates = 0;
    long wiredUpdateCopies = 0;

    omnetpp::simsignal_t registrationRequestsSignal;
    omnetpp::simsignal_t registrationResponsesSignal;
    omnetpp::simsignal_t keyUpdatesSignal;

    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    virtual void finish() override;

    std::string generateGroupKey();
    void processRegistration(veins::TraCIDemo11pMessage *msg);
    void sendRegistrationResponse(const std::string& vehicleId, int rsuIndex);
    void performKeyUpdate();
//    void sendKeyUpdateToAllRsus();
    void sendKeyUpdateToAllRsus(const std::string&);
//    void sendKeyUpdateToAllRsus(const bn_t x_r, const ep2_t A_hat);
  public:
    virtual ~GroupKeyTA() override = default;
};

} // namespace groupkey
