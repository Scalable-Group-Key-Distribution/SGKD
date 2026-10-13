#include "GroupKeyTA.h"
#include <iomanip>
#include <sstream>
#include <random>
#include <functional>
#include "SGKD.h"
#include "SGKDSerializer.h"
using namespace omnetpp;
using namespace veins;

namespace gkd_vanet {

Define_Module(GroupKeyTA);

void GroupKeyTA::initialize()
{
    registrationRequestsSignal = registerSignal("registrationRequests");
    registrationResponsesSignal = registerSignal("registrationResponses");
    keyUpdatesSignal = registerSignal("keyUpdates");

    currentGroupKey = generateGroupKey();
    currentKeyVersion = 0;

    EV_INFO << "TA initialized with group-key version " << currentKeyVersion << "\n";
    EV_INFO << "Initial simulated group key: " << currentGroupKey << "\n";

    keyUpdateTimer = new cMessage("keyUpdateTimer");
    scheduleAt(simTime() + par("keyUpdateInterval"), keyUpdateTimer);

    if(!SGKDEngine::init())
    {  EV_INFO<<"Relic Library not Initialized Correctly";
    return;
    }
    ep_null(PP.g1); ep2_null(PP.g2);ep2_null(PP.A); ep_new(PP.g1); ep2_new(PP.g2);ep2_new(PP.A);
    ep_null(SSecrets.h); ep_new(SSecrets.h);
    bn_null(SSecrets.sk); bn_new(SSecrets.sk);
    SGKDEngine::Setup(PP, SSecrets);
    EV_INFO <<"TA   INitialization completed"<<endl;
    EV_INFO << "[+] g1:"<< serializeToHex(PP.g1) << endl;
    EV_INFO << "[+] g2:"<<serializeToHex(PP.g2)  << endl;
    EV_INFO << "[+] A:"<<serializeToHex(PP.A) << endl;
    EV_INFO << "[+] sk:"<<serializeToHex(SSecrets.sk) << endl;
    EV_INFO << "[+] h:"<<serializeToHex(SSecrets.h) << endl;

}

std::string GroupKeyTA::generateGroupKey()
{
    std::ostringstream out;
    std::string material = std::to_string(currentKeyVersion) + ":" +
                            std::to_string(simTime().raw());
    const auto value = std::hash<std::string>{}(material);
    out << "GK-" << currentKeyVersion << "-" << std::hex << value;
    return out.str();
}

void GroupKeyTA::handleMessage(cMessage *msg)
{
    // Fix 1: Check for self-message before inspecting arrival gate
    if (msg->isSelfMessage()) {
        if (msg == keyUpdateTimer) {
            EV_INFO << "TA: key update timer fired at t=" << simTime() << endl;
            performKeyUpdate();
            scheduleAt(simTime() + par("keyUpdateInterval"), keyUpdateTimer);
        } else if(strcmp(msg->getName(), "sendRegistrationResponse")==0)
         {
            sendRegistrationResponse(msg->par("payLoad").stringValue(), msg->par("rsuIndex").longValue());
            EV_INFO<<"TA handle sendRegistration Response"<<endl;
            EV_INFO<<"PayLoad:"<<msg->par("payLoad").stringValue()<<endl;
            EV_INFO<<"rsuIndex"<<msg->par("rsuIndex").longValue()<<endl;
         }
        else if(strcmp(msg->getName(), "sendKeyUpdate")==0)
        {
            sendKeyUpdateToAllRsus(msg->par("payLoad").stringValue());
            EV_INFO<<"TA handle sendKeyUpdate"<<endl;
            EV_INFO<<"PayLoad:"<<msg->par("payLoad").stringValue()<<endl;

        }
        else {
            delete msg;
        }
        return;
    }

    EV_INFO << "TA: handleMessage called for external message, msg="
            << msg->getName()
            << ", arrivalGate="
            << (msg->getArrivalGate() ? msg->getArrivalGate()->getFullName() : "NONE")
            << endl;

    auto *packet = dynamic_cast<TraCIDemo11pMessage *>(msg);

    if (!packet) {
        EV_ERROR << "TA: received message is NOT TraCIDemo11pMessage" << endl;
        delete msg;
        return;
    }

    EV_INFO << "TA: received data = " << packet->getDemoData() << endl;

    const std::string data = packet->getDemoData();

    if (data.rfind("REG|", 0) == 0) {
        EV_INFO << "TA: processing registration" << endl;
        processRegistration(packet);
        return;
    }

    EV_INFO << "TA: unknown message, deleting" << endl;
    delete packet;
}

void GroupKeyTA::processRegistration(TraCIDemo11pMessage *msg)
{
    EV_INFO << "TA received registration: "
            << msg->getDemoData()
            << " from RSU "
            << msg->getSenderAddress()
            << "\n";
    registrationRequests++;
    emit(registrationRequestsSignal, registrationRequests);

    //start recording the cpu time
    auto start = std::chrono::high_resolution_clock::now();
    std::stringstream ss(msg->getDemoData());
    std::string type, vehicleId;
    int ignoredRsu = -1;
    std::getline(ss, type, '|');
    std::getline(ss, vehicleId, '|');
    ss >> ignoredRsu;

    int rsuIndex = msg->getSenderAddress();
    if (rsuIndex < 0 || rsuIndex >= gateSize("wiredOut")) {
        EV_WARN << "Registration received without a valid RSU index: " << rsuIndex << "\n";
        delete msg;
        return;
    }

    if (registeredVehicles.insert(vehicleId).second) {
        EV_INFO << "Registering vehicle " << vehicleId << " through RSU " << rsuIndex << "\n";
    }
    //assign secrts for the vehicle
     VehicleSecrets v_sec;
    bn_null(v_sec.x_i); bn_new(v_sec.x_i);
    ep_null(v_sec.w_i1); ep2_null(v_sec.w_i2); ep_null(v_sec.w_i3);
    ep_new(v_sec.w_i1);  ep2_new(v_sec.w_i2);  ep_new(v_sec.w_i3);

        SGKDEngine::AddMember(PP, SSecrets, v_sec);
        EV_INFO<<"Provisioned Vehicle Secrets"<<endl;
        EV_INFO<<"x_i:"<<serializeToHex(v_sec.x_i)<<endl;
        EV_INFO<<"w_i1:"<<serializeToHex(v_sec.w_i1)<<endl;
        EV_INFO<<"w_i2:"<<serializeToHex(v_sec.w_i2)<<endl;
        EV_INFO<<"w_i3:"<<serializeToHex(v_sec.w_i3)<<endl;
        std::string responseData =
                "RESP|" + vehicleId + "|" +
                std::to_string(currentKeyVersion) + "|" +serializeToHex(v_sec.x_i)+ "|" +serializeToHex(v_sec.w_i1)+ "|" +serializeToHex(v_sec.w_i2)+ "|" +serializeToHex(v_sec.w_i3)+"|"+serializeToHex(PP.A);
      // recorrd the end cpu time
        auto end = std::chrono::high_resolution_clock::now();
            // Calculate CPU duration in seconds
            double cpuTimeSec = std::chrono::duration<double>(end - start).count();
         EV_INFO<<"Request Processing Latency="<<cpuTimeSec<<endl;
         recordScalar("requestProcessingLatency", cpuTimeSec);

     // schedule sending the registration response after the current simulation time+the Cpu time for the registration request processing
        cMessage* sendRegistrationResponse= new cMessage("sendRegistrationResponse");
        sendRegistrationResponse->addPar("payLoad").setStringValue(responseData.c_str());
        sendRegistrationResponse->addPar("rsuIndex").setLongValue(rsuIndex);
        scheduleAt(simTime()+cpuTimeSec, sendRegistrationResponse);
//    sendRegistrationResponse(vehicleId, rsuIndex);
    delete msg;
}

void GroupKeyTA::sendRegistrationResponse(const std::string& responseData, int rsuIndex)
{
    if (!gate("wiredOut", rsuIndex)->isConnected()) {
        EV_WARN << "TA: Gate wiredOut[" << rsuIndex << "] is not connected!\n";
        return;
    }

    EV_INFO << "TA sending registration response to RSU " << rsuIndex << "\n";
    auto *response = new TraCIDemo11pMessage("RegistrationResponse");

     response->setDemoData(responseData.c_str());
    response->setSenderAddress(rsuIndex);
    response->setByteLength(responseData.length());

    send(response, "wiredOut", rsuIndex);

    registrationResponses++;
    emit(registrationResponsesSignal, registrationResponses);
}

void GroupKeyTA::performKeyUpdate()
{
    currentKeyVersion++;
//    currentGroupKey = generateGroupKey();
    //start recording the cpu time
       auto start = std::chrono::high_resolution_clock::now();
    bn_t x_r;
    bn_null(x_r); bn_new(x_r);
    SGKDEngine::UpdateGroupKey(PP, SSecrets,x_r);
    std::string updateData =
                "UPDATE|" +
                std::to_string(currentKeyVersion) + "|" + serializeToHex(x_r)+"|"+serializeToHex(PP.A)+"|"+simTime().str();
    // recorrd the end cpu time
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate CPU duration in seconds
    double cpuTimeSec = std::chrono::duration<double>(end - start).count();
     EV_INFO<<"Update Generation Latency="<<cpuTimeSec<<endl;
     recordScalar("updateGenerationLatency", cpuTimeSec);
    keyUpdates++;
    emit(keyUpdatesSignal, keyUpdates);

    EV_INFO << "TA generated group-key version " << currentKeyVersion << "\n";

    cMessage* sendKeyUpdate=new cMessage("sendKeyUpdate");
    sendKeyUpdate->addPar("payLoad").setStringValue(updateData.c_str());
    scheduleAt(simTime()+cpuTimeSec, sendKeyUpdate);
//    sendKeyUpdateToAllRsus(x_r,PP.A);
    bn_clean(x_r);
}
void GroupKeyTA::sendKeyUpdateToAllRsus(const std::string& updateData)
{
    int numGates = gateSize("wiredOut");
    for (int i = 0; i < numGates; ++i) {
        // Fix 2: Verify gate connectivity before sending
        if (!gate("wiredOut", i)->isConnected()) {
            EV_WARN << "TA: skipping unconnected gate wiredOut[" << i << "]\n";
            continue;
        }

        auto *update = new TraCIDemo11pMessage("KeyUpdate");
        update->setDemoData(updateData.c_str());
        update->setSenderAddress(i);
        update->setByteLength(updateData.length());

        send(update, "wiredOut", i);
        wiredUpdateCopies++;
    }
}
//void GroupKeyTA::sendKeyUpdateToAllRsus(const bn_t x_r, const ep2_t A_hat)
//{
//    int numGates = gateSize("wiredOut");
//    for (int i = 0; i < numGates; ++i) {
//        // Fix 2: Verify gate connectivity before sending
//        if (!gate("wiredOut", i)->isConnected()) {
//            EV_WARN << "TA: skipping unconnected gate wiredOut[" << i << "]\n";
//            continue;
//        }
//
//        auto *update = new TraCIDemo11pMessage("KeyUpdate");
//
//        std::string updateData =
//            "UPDATE|" +
//            std::to_string(currentKeyVersion) + "|" + serializeToHex(x_r)+"|"+serializeToHex(A_hat)+"|"+simTime().str();
//        update->setDemoData(updateData.c_str());
//        update->setSenderAddress(i);
//        update->setByteLength(updateData.length());
//
//        send(update, "wiredOut", i);
//        wiredUpdateCopies++;
//    }
//}
//void GroupKeyTA::sendKeyUpdateToAllRsus()
//{
//    int numGates = gateSize("wiredOut");
//    for (int i = 0; i < numGates; ++i) {
//        // Fix 2: Verify gate connectivity before sending
//        if (!gate("wiredOut", i)->isConnected()) {
//            EV_WARN << "TA: skipping unconnected gate wiredOut[" << i << "]\n";
//            continue;
//        }
//
//        auto *update = new TraCIDemo11pMessage("KeyUpdate");
//
//        std::string updateData =
//            "UPDATE|" +
//            std::to_string(currentKeyVersion) + "|" +
//            currentGroupKey;
//
//        update->setDemoData(updateData.c_str());
//        update->setSenderAddress(i);
//        update->setByteLength(64);
//
//        send(update, "wiredOut", i);
//        wiredUpdateCopies++;
//    }
//}

void GroupKeyTA::finish()
{
    recordScalar("registeredVehicles", static_cast<double>(registeredVehicles.size()));
    recordScalar("registrationRequests", registrationRequests);
    recordScalar("registrationResponses", registrationResponses);
    recordScalar("keyUpdates", keyUpdates);
    recordScalar("wiredUpdateCopies", wiredUpdateCopies);

    if (keyUpdateTimer) {
        cancelAndDelete(keyUpdateTimer);
        keyUpdateTimer = nullptr;
    }
}

} // namespace gkd_vanet
//#include "GroupKeyTA.h"
//#include <iomanip>
//#include <sstream>
//#include <random>
//#include <functional>
//
//using namespace omnetpp;
//using namespace veins;
//
//namespace gkd_vanet {
//
//Define_Module(GroupKeyTA);
//
//void GroupKeyTA::initialize()
//{
//    registrationRequestsSignal = registerSignal("registrationRequests");
//    registrationResponsesSignal = registerSignal("registrationResponses");
//    keyUpdatesSignal = registerSignal("keyUpdates");
//
//    currentGroupKey = generateGroupKey();
//    currentKeyVersion = 0;
//
//    EV_INFO << "TA initialized with group-key version " << currentKeyVersion << "\n";
//    EV_INFO << "Initial simulated group key: " << currentGroupKey << "\n";
//
//    keyUpdateTimer = new cMessage("keyUpdateTimer");
//    scheduleAt(simTime() + par("keyUpdateInterval"), keyUpdateTimer);
//}
//
//std::string GroupKeyTA::generateGroupKey()
//{
//    // Simulation placeholder only. Replace this function with the key-generation
//    // procedure of the proposed cryptographic group-key scheme.
//    std::ostringstream out;
//    std::string material = std::to_string(currentKeyVersion) + ":" +
//                            std::to_string(simTime().raw());
//    const auto value = std::hash<std::string>{}(material);
//    out << "GK-" << currentKeyVersion << "-" << std::hex << value;
//    return out.str();
//}
//
////void GroupKeyTA::handleMessage(cMessage *msg)
////{
////    EV_INFO<<"t:"<<simTime()<<" [TA "<<getId()<<"] handling message"<<endl;
////
////    if (msg == keyUpdateTimer) {
////        EV_INFO<<"t:"<<simTime()<<" [TA "<<getId()<<"] handling keyupdate"<<endl;
////
////        performKeyUpdate();
////        scheduleAt(simTime() + par("keyUpdateInterval"), keyUpdateTimer);
////        return;
////    }
////
////    auto *packet = dynamic_cast<TraCIDemo11pMessage *>(msg);
////    if (!packet) {
////        delete msg;
////        delete packet;
////        return;
////    }
////
////    const std::string data = packet->getDemoData();
////    if (data.rfind("REG|", 0) == 0) {
////        processRegistration(packet);
////    }
////    delete msg;
////    delete packet;
////
////}
//void GroupKeyTA::handleMessage(cMessage *msg)
//{
//    EV_INFO << "TA: handleMessage called, msg="
//            << msg->getName()
//            << ", id=" << msg->getId()
//            << ", arrivalGate="
//            << msg->getArrivalGate()->getFullName()
//            << endl;
//
//    if (msg == keyUpdateTimer) {
//        EV_INFO << "TA: key update timer fired" << endl;
//
//        performKeyUpdate();
//        scheduleAt(simTime() + par("keyUpdateInterval"), keyUpdateTimer);
//        return;
//    }
//
//    EV_INFO << "TA: received non-timer message" << endl;
//
//    auto *packet = dynamic_cast<TraCIDemo11pMessage *>(msg);
//
//    if (!packet) {
//        EV_ERROR << "TA: received message is NOT TraCIDemo11pMessage"
//                 << endl;
//        delete msg;
//        return;
//    }
//
//    EV_INFO << "TA: received data = "
//            << packet->getDemoData()
//            << endl;
//
//    const std::string data = packet->getDemoData();
//
//    if (data.rfind("REG|", 0) == 0) {
//        EV_INFO << "TA: processing registration" << endl;
//        processRegistration(packet);
//        return;
//    }
//
//    EV_INFO << "TA: unknown message, deleting" << endl;
//    delete packet;
//}
//void GroupKeyTA::processRegistration(TraCIDemo11pMessage *msg)
//{
//    EV_INFO << "TA received registration: "
//            << msg->getDemoData()
//            << " from RSU "
//            << msg->getSenderAddress()
//            << "\n";
//    registrationRequests++;
//    emit(registrationRequestsSignal, registrationRequests);
//
//    std::stringstream ss(msg->getDemoData());
//    std::string type, vehicleId;
//    int ignoredRsu = -1;
//    std::getline(ss, type, '|');
//    std::getline(ss, vehicleId, '|');
//    ss >> ignoredRsu;
//
//    int rsuIndex = msg->getSenderAddress();
//    if (rsuIndex < 0 || rsuIndex >= gateSize("wiredOut")) {
//        EV_WARN << "Registration received without a valid RSU index\n";
//        delete msg;
//        return;
//    }
//
//    if (registeredVehicles.insert(vehicleId).second) {
//        EV_INFO << "Registering vehicle " << vehicleId << " through RSU " << rsuIndex << "\n";
//    }
//    EV_INFO << "TA sending registration response to RSU "
//            << rsuIndex << "\n";
//    sendRegistrationResponse(vehicleId, rsuIndex);
//    delete msg;
//}
//
////void GroupKeyTA::sendRegistrationResponse(const std::string& vehicleId, int rsuIndex)
////{
////    auto *response = new TraCIDemo11pMessage("RegistrationResponse");
////    response->setDemoData("RESP|" + vehicleId + "|" + std::to_string(currentKeyVersion) + "|" + currentGroupKey);
////    response->setSenderAddress(rsuIndex);
////    response->setByteLength(64);
////    send(response, "wiredOut", rsuIndex);
////
////    registrationResponses++;
////    emit(registrationResponsesSignal, registrationResponses);
////}
//void GroupKeyTA::sendRegistrationResponse(const std::string& vehicleId, int rsuIndex)
//{
//    EV_INFO << "TA sending registration response to RSU "
//            << rsuIndex << "\n";
//    auto *response = new TraCIDemo11pMessage("RegistrationResponse");
//
//    std::string responseData =
//        "RESP|" + vehicleId + "|" +
//        std::to_string(currentKeyVersion) + "|" +
//        currentGroupKey;
//
//    response->setDemoData(responseData.c_str());
//    response->setSenderAddress(rsuIndex);
//    response->setByteLength(64);
//
//    send(response, "wiredOut", rsuIndex);
//
//    registrationResponses++;
//    emit(registrationResponsesSignal, registrationResponses);
//}
//
//
//void GroupKeyTA::performKeyUpdate()
//{
//    currentKeyVersion++;
//    currentGroupKey = generateGroupKey();
//    keyUpdates++;
//    emit(keyUpdatesSignal, keyUpdates);
//
//    EV_INFO << "TA generated group-key version " << currentKeyVersion << "\n";
//    sendKeyUpdateToAllRsus();
//}
//
////void GroupKeyTA::sendKeyUpdateToAllRsus()
////{
////    for (int i = 0; i < gateSize("wiredOut"); ++i) {
////        auto *update = new TraCIDemo11pMessage("KeyUpdate");
////        update->setDemoData("UPDATE|" + std::to_string(currentKeyVersion) + "|" + currentGroupKey);
////        update->setSenderAddress(i);
////        update->setByteLength(64);
////        send(update, "wiredOut", i);
////        wiredUpdateCopies++;
////    }
////}
//void GroupKeyTA::sendKeyUpdateToAllRsus()
//{
//    for (int i = 0; i < gateSize("wiredOut"); ++i) {
//        auto *update = new TraCIDemo11pMessage("KeyUpdate");
//
//        std::string updateData =
//            "UPDATE|" +
//            std::to_string(currentKeyVersion) + "|" +
//            currentGroupKey;
//
//        update->setDemoData(updateData.c_str());
//        update->setSenderAddress(i);
//        update->setByteLength(64);
//
//        send(update, "wiredOut", i);
//        wiredUpdateCopies++;
//    }
//}
//void GroupKeyTA::finish()
//{
//    recordScalar("registeredVehicles", static_cast<double>(registeredVehicles.size()));
//    recordScalar("registrationRequests", registrationRequests);
//    recordScalar("registrationResponses", registrationResponses);
//    recordScalar("keyUpdates", keyUpdates);
//    recordScalar("wiredUpdateCopies", wiredUpdateCopies);
//    cancelAndDelete(keyUpdateTimer);
//    keyUpdateTimer = nullptr;
//}
//
//} // namespace groupkey
