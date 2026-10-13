#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace gkd_vanet {

class GroupKeyRSUApp : public veins::DemoBaseApplLayer {
  protected:
    long registrationsRelayed = 0;
    long responsesRelayed = 0;
    long keyUpdatesBroadcast = 0;
    long updateVehiclesForwarded = 0;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void onWSM(veins::BaseFrame1609_4 *wsm) override;
    virtual void handleSelfMsg(cMessage *msg) override;
    virtual void finish() override;

    void processWired(veins::TraCIDemo11pMessage *msg);
    void forwardRegistrationToTA(veins::TraCIDemo11pMessage *msg);
    void forwardResponseToVehicle(veins::TraCIDemo11pMessage *msg);
    void broadcastUpdate(veins::TraCIDemo11pMessage *msg);
};

} // namespace groupkey
