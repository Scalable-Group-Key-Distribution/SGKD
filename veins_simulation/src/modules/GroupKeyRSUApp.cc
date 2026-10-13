#include "GroupKeyRSUApp.h"

using namespace omnetpp;
using namespace veins;

namespace gkd_vanet {

Define_Module(GroupKeyRSUApp);

void GroupKeyRSUApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
}

void GroupKeyRSUApp::handleMessage(cMessage *msg)
{
    EV_INFO << "RSU " << getParentModule()->getIndex()
              << " received message: \n";
    // Wired packets enter through the application-specific gate.
    if (msg->getArrivalGate() == gate("wiredIn")) {
        processWired(check_and_cast<TraCIDemo11pMessage *>(msg));
        return;
    }
    EV_INFO << "RSU " << getParentModule()->getIndex()
                 << " received message (not wired): \n";

//   delete msg;
    DemoBaseApplLayer::handleMessage(msg);
}

//void GroupKeyRSUApp::processWired(TraCIDemo11pMessage *msg)
//{
//    const std::string data = msg->getDemoData();
//
//    if (data.rfind("RESP|", 0) == 0) {
//        forwardResponseToVehicle(msg);
//    }
//    else if (data.rfind("UPDATE|", 0) == 0) {
//        broadcastUpdate(msg);
//    }
//    else {
//        delete msg;
//    }
//}

void GroupKeyRSUApp::processWired(TraCIDemo11pMessage *msg)
{
    EV_INFO << "RSU " << getParentModule()->getIndex()
            << " received wired message: "
            << msg->getDemoData()
            << "\n";
    const std::string data = msg->getDemoData();

    if (data.rfind("RESP|", 0) == 0) {
        forwardResponseToVehicle(msg);
        return;
    }

    else if (data.rfind("UPDATE|", 0) == 0) {
        broadcastUpdate(msg);
        return;
    }
    else
    {
    delete msg;
    }
}

//void GroupKeyRSUApp::onWSM(BaseFrame1609_4 *frame)
//{
//    auto *msg = check_and_cast<TraCIDemo11pMessage *>(frame);
//    const std::string data = msg->getDemoData();
//
//    if (data.rfind("REG|", 0) == 0) {
//        forwardRegistrationToTA(msg);
//    }
//    else {
//        delete msg;
//    }
//}
void GroupKeyRSUApp::onWSM(BaseFrame1609_4 *frame)
{
    EV_INFO << "RSU " << getParentModule()->getIndex()
              << " received message: \n";
    auto *msg = check_and_cast<TraCIDemo11pMessage *>(frame);

    const std::string data = msg->getDemoData();
    EV_INFO<<"[RSU"<<getId()<<"] data"<<data<<endl;

    if (data.rfind("REG|", 0) == 0) {
        forwardRegistrationToTA(msg);
        return;   // IMPORTANT: ownership transferred by send()
    }
//    else
//    {   delete msg;}
}

//void GroupKeyRSUApp::forwardRegistrationToTA(TraCIDemo11pMessage *msg)
//{
//    // Use the parent RSU array index as the RSU identifier. This is populated
//    // by the network's RSU[i] submodule at initialization.
//    EV_INFO<<"t:"<<simTime()<<" [RSU "<<getId()<<"] forwarding registration request to TA"<<endl;
//    int rsuIndex = getParentModule()->getIndex();
//    msg->setSenderAddress(rsuIndex);
//    msg->setByteLength(64);
//    send(msg, "wiredOut");
//    registrationsRelayed++;
//    EV_INFO<<"t:"<<simTime()<<" [RSU "<<getId()<<"] forwarded registration request to TA"<<endl;
//
//}
void GroupKeyRSUApp::forwardRegistrationToTA(TraCIDemo11pMessage *msg)
{
    EV_INFO << "t:" << simTime()
            << " [RSU " << getId()
            << "] forwarding registration request to TA" << endl;

    int rsuIndex = getParentModule()->getIndex();

    // Create a new packet for the wired TA-RSU link.
    auto *wiredMsg = new TraCIDemo11pMessage("RegistrationToTA");

    wiredMsg->setDemoData(msg->getDemoData());
    wiredMsg->setSenderAddress(rsuIndex);
    wiredMsg->setByteLength(64);

    // The wireless packet has now been consumed.
//    delete msg;

    EV_INFO << "t:" << simTime()
            << " [RSU " << getId()
            << "] sending registration to TA through wiredOut" << endl;

    EV_INFO << "wiredOut connected = "
            << gate("wiredOut")->isConnected()
            << endl;

    send(wiredMsg, "wiredOut");

    registrationsRelayed++;

    EV_INFO << "t:" << simTime()
            << " [RSU " << getId()
            << "] forwarded registration request to TA" << endl;
}

//void GroupKeyRSUApp::forwardResponseToVehicle(TraCIDemo11pMessage *msg)
//{
//    EV_INFO << "RSU " << getParentModule()->getIndex()
//            << " forwarding response to vehicle: "
//            << msg->getDemoData()
//            << "\n";
//    // demoData is RESP|vehicleId|version|key. Preserve the payload and use
//    // broadcast at the WAVE layer; vehicles accept only responses addressed
//    // to their own identifier. This avoids needing a preconfigured RSU MAC.
//    populateWSM(msg);
//    msg->setSenderAddress(myId);
//    msg->setRecipientAddress(-1);
//    msg->setByteLength(64);
//    sendDown(msg);
//    responsesRelayed++;
//}
void GroupKeyRSUApp::forwardResponseToVehicle(TraCIDemo11pMessage *msg)
{
    EV_INFO << "RSU " << getParentModule()->getIndex()
            << " forwarding response to vehicles: "
            << msg->getDemoData() << endl;
    EV_INFO<< "RSU sending"<<msg->getDemoData()<<endl;
    auto *wirelessMsg =
        new TraCIDemo11pMessage("RegistrationResponse");

    wirelessMsg->setDemoData(msg->getDemoData());

    populateWSM(wirelessMsg);

    wirelessMsg->setSenderAddress(myId);
    wirelessMsg->setRecipientAddress(-1);
    wirelessMsg->setByteLength(64);

    // The wired packet has been consumed.
    delete msg;

    EV_INFO << "RSU " << getParentModule()->getIndex()
            << " sending new response packet down 802.11p" << endl;

    sendDown(wirelessMsg);
    EV_INFO<<"RSU sent"<<wirelessMsg->getDemoData()<<endl;
    responsesRelayed++;
}
void GroupKeyRSUApp::broadcastUpdate(TraCIDemo11pMessage *msg)
{
    populateWSM(msg);
    msg->setSenderAddress(myId);
    msg->setRecipientAddress(-1);
    msg->setByteLength(64);
    sendDown(msg);
    keyUpdatesBroadcast++;
}

void GroupKeyRSUApp::handleSelfMsg(cMessage *msg)
{
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void GroupKeyRSUApp::finish()
{
    recordScalar("registrationsRelayed", registrationsRelayed);
    recordScalar("responsesRelayed", responsesRelayed);
    recordScalar("keyUpdatesBroadcast", keyUpdatesBroadcast);
    recordScalar("updateVehiclesForwarded", updateVehiclesForwarded);
    DemoBaseApplLayer::finish();
}

} // namespace groupkey
