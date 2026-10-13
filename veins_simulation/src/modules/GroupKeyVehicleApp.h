#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "SGKD.h"
#include "SGKDSerializer.h"
namespace gkd_vanet {

class GroupKeyVehicleApp : public veins::DemoBaseApplLayer {
  protected:
    omnetpp::cMessage *registrationTimer = nullptr;
    std::string vehicleId;
    std::string currentGroupKey;
    int currentKeyVersion = -1;
    VehicleSecrets VSecrets;
    gt_t CGKey;
    ep2_t currentA;
    bool isregistered=false;
    long registrationAttempts = 0;
    long registrationSuccesses = 0;
    long keyUpdatesReceived = 0;
    long staleUpdates = 0;
    long missedUpdates = 0;
    omnetpp::simtime_t registrationStart = -1;
    omnetpp::simtime_t registrationLatency = 0;
    omnetpp::simtime_t updateStart=-1;
    omnetpp::simtime_t updateLatency=0;
    long updateAttempts=0;
    long updateSuccesses=0;
    omnetpp::simsignal_t registrationLatencySignal;
    omnetpp::simsignal_t keyUpdateVersionSignal;
    omnetpp::simsignal_t updateLatencySignal;
    virtual void initialize(int stage) override;
    virtual void onWSM(veins::BaseFrame1609_4 *wsm) override;
    virtual void handleSelfMsg(omnetpp::cMessage *msg) override;
    virtual void finish() override;
//    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void sendRegistrationRequest();
    void processRegistrationResponse(const std::string& data);
    void processKeyUpdate(const std::string& data);
};

} // namespace groupkey
