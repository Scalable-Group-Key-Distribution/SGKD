#include "GroupKeyVehicleApp.h"

using namespace omnetpp;
using namespace veins;

namespace gkd_vanet {

Define_Module(GroupKeyVehicleApp);

//void GroupKeyVehicleApp::initialize(int stage)
//{
//    DemoBaseApplLayer::initialize(stage);
//
//    if (stage == 0) {
//        vehicleId = getParentModule()->getFullName();
//        registrationLatencySignal = registerSignal("registrationLatency");
//        keyUpdateVersionSignal = registerSignal("keyUpdateVersion");
//
//        registrationTimer = new cMessage("registrationTimer");
//        scheduleAt(simTime() + uniform(0.5, 2.0), registrationTimer);
//    }
//}
void GroupKeyVehicleApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        EV_INFO << "========================================" << endl;
        EV_INFO << "GroupKeyVehicleApp INITIALIZED" << endl;
        EV_INFO << "module = " << getFullPath() << endl;
        EV_INFO << "parent = " << getParentModule()->getFullPath() << endl;
        EV_INFO << "========================================" << endl;
        EV_INFO << "lowerLayerIn = "
                << gate("lowerLayerIn")->getFullPath() << endl;

        EV_INFO << "lowerLayerOut = "
                << gate("lowerLayerOut")->getFullPath() << endl;

        EV_INFO << "lowerControlIn = "
                << gate("lowerControlIn")->getFullPath() << endl;

        EV_INFO << "lowerControlOut = "
                << gate("lowerControlOut")->getFullPath() << endl;

        EV_INFO << "lowerLayerIn connected = "
                << gate("lowerLayerIn")->isConnected() << endl;

        EV_INFO << "lowerLayerOut connected = "
                << gate("lowerLayerOut")->isConnected() << endl;
        vehicleId = getParentModule()->getFullName();

        registrationLatencySignal =
            registerSignal("registrationLatency");

        keyUpdateVersionSignal =
            registerSignal("keyUpdateVersion");
        updateLatencySignal=registerSignal("updateLatency");
        registrationTimer =
            new cMessage("registrationTimer");

        scheduleAt(simTime() + uniform(0.5, 2.0),
                   registrationTimer);
        if(!SGKDEngine::init())
        {
            EV_INFO<<"V"<<getParentModule()->getIndex()<<" Relic not initialized correctly"<<endl;
        }
        //initialize secrets
        bn_null(VSecrets.x_i); bn_new(VSecrets.x_i);
        ep_null(VSecrets.w_i1); ep2_null(VSecrets.w_i2); ep_null(VSecrets.w_i3);
        ep_new(VSecrets.w_i1);  ep2_new(VSecrets.w_i2);  ep_new(VSecrets.w_i3);
        gt_null(CGKey); gt_new(CGKey);
        ep2_null(currentA); ep2_new(currentA);

//        EV_INFO << "========================================================\n";
//            EV_INFO << "Running Bilinear Pairing Benchmark (10000iterations)...\n";
//            EV_INFO << "========================================================\n";
//
//        double avgMs=0;
//        simtime_t pcstart=simTime();
//        SGKDEngine::benchmarkPairing(avgMs,1000);
//        EV_INFO << "Average Pairing Latency: " << avgMs << " ms\n";
//        EV_INFO << "Average Pairing Latency/simtime: " << (simTime()-pcstart).str() << "\n";
    }
}
//void GroupKeyVehicleApp::handleMessage(cMessage *msg)
//{
//    EV_INFO << "========================================" << endl;
//    EV_INFO << "VEHICLE APP handleMessage CALLED" << endl;
//    EV_INFO << "message = " << msg->getName() << endl;
//    EV_INFO << "message type = " << msg->getClassName() << endl;
//    EV_INFO << "arrival gate = "
//            << msg->getArrivalGate()->getFullPath() << endl;
//    EV_INFO << "========================================" << endl;
//    if (msg == registrationTimer) {
//            sendRegistrationRequest();
//            return;
//       }
////    delete msg;
//    DemoBaseApplLayer::handleMessage(msg);
//}

//void GroupKeyVehicleApp::handleSelfMsg(cMessage *msg)
//{
//    if (msg == registrationTimer) {
//        sendRegistrationRequest();
//        return;
//    }
////    delete msg;
//    DemoBaseApplLayer::handleSelfMsg(msg);
//}
void GroupKeyVehicleApp::handleSelfMsg(cMessage *msg)
{
    EV_INFO << "VEHICLE: handleSelfMsg() called with "
            << msg->getName() << endl;

    if (msg == registrationTimer) {
        EV_INFO << "VEHICLE: registration timer fired" << endl;
        sendRegistrationRequest();
        return;
    }

    EV_INFO << "VEHICLE: unexpected self-message: "
            << msg->getName() << endl;

    DemoBaseApplLayer::handleSelfMsg(msg);
}

//void GroupKeyVehicleApp::sendRegistrationRequest()
//{
//    auto *msg = new TraCIDemo11pMessage("RegistrationRequest");
//    populateWSM(msg);
//    msg->setRecipientAddress(-1);
//    msg->setDemoData("REG|" + vehicleId + "|-1");
//    msg->setByteLength(64);
//    registrationStart = simTime();
//    registrationAttempts++;
//    sendDown(msg);
//}
//void GroupKeyVehicleApp::sendRegistrationRequest()
//{
//    auto *msg = new TraCIDemo11pMessage("RegistrationRequest");
//    populateWSM(msg);
//
//    msg->setRecipientAddress(-1);
//
//    std::string registrationData =
//        "REG|" + vehicleId + "|-1";
//
//    msg->setDemoData(registrationData.c_str());
//    msg->setByteLength(64);
//
//    registrationStart = simTime();
//    registrationAttempts++;
//
//    sendDown(msg);
//}
void GroupKeyVehicleApp::sendRegistrationRequest()
{
    EV_INFO << "VEHICLE: entering sendRegistrationRequest()" << endl;
    registrationStart = simTime();
     registrationAttempts++;

    auto *msg = new TraCIDemo11pMessage("RegistrationRequest");

    EV_INFO << "VEHICLE: message created" << endl;

    populateWSM(msg);

    EV_INFO << "VEHICLE: populateWSM completed" << endl;

    msg->setRecipientAddress(-1);

    EV_INFO << "VEHICLE: recipient set" << endl;

    std::string registrationData =
        "REG|" + vehicleId + "|-1";

    msg->setDemoData(registrationData.c_str());

    EV_INFO << "VEHICLE: demo data set: "
            << registrationData << endl;

    msg->setByteLength(64);

    EV_INFO << "VEHICLE: byte length set" << endl;


    EV_INFO << "VEHICLE: about to sendDown()" << endl;

    sendDown(msg);

    EV_INFO << "VEHICLE: sendDown() completed" << endl;
}

//void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
//{
//    auto *msg = check_and_cast<TraCIDemo11pMessage *>(frame);
//    const std::string data = msg->getDemoData();
//
//    if (data.rfind("RESP|", 0) == 0) {
//        processRegistrationResponse(data);
//    }
//    else if (data.rfind("UPDATE|", 0) == 0) {
//        processKeyUpdate(data);
//    }
//    else {
//        delete msg;
//    }
//}

//void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
//{
//    auto *msg = check_and_cast<TraCIDemo11pMessage *>(frame);
//    const std::string data = msg->getDemoData();
//
//    if (data.rfind("RESP|", 0) == 0) {
//        processRegistrationResponse(data);
//    }
//    else if (data.rfind("UPDATE|", 0) == 0) {
//        processKeyUpdate(data);
//    }
//
//    delete msg;
//}
//void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
//{
//    EV_INFO << "VEHICLE: onWSM() called" << endl;
//
//    auto *msg = check_and_cast<TraCIDemo11pMessage *>(frame);
//
//    const std::string data = msg->getDemoData();
//
//    EV_INFO << "VEHICLE: received packet: "
//            << data << endl;
//
//    if (data.rfind("RESP|", 0) == 0) {
//        EV_INFO << "VEHICLE: processing registration response" << endl;
//
//        processRegistrationResponse(data);
//
//        delete msg;
//        return;
//    }
//
//    if (data.rfind("UPDATE|", 0) == 0) {
//        EV_INFO << "VEHICLE: processing group-key update" << endl;
//
//        processKeyUpdate(data);
//
//        delete msg;
//        return;
//    }
//
//    EV_INFO << "VEHICLE: unknown wireless message" << endl;
//
////    delete msg;
//}

//void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
//{
//    EV_INFO << "VEHICLE: onWSM() called" << endl;
//
//    auto *msg = dynamic_cast<TraCIDemo11pMessage *>(frame);
//
//    if (msg == nullptr) {
//        EV_WARN << "VEHICLE: received WSM is not "
//                   "TraCIDemo11pMessage" << endl;
//        delete frame;
//        return;
//    }
//
//    const std::string data = msg->getDemoData();
//
//    EV_INFO << "VEHICLE: received packet: "
//            << data << endl;
//
//    if (data.rfind("RESP|", 0) == 0) {
//        EV_INFO << "VEHICLE: processing registration response" << endl;
//        processRegistrationResponse(data);
//        delete msg;
//        return;
//    }
//
//    if (data.rfind("UPDATE|", 0) == 0) {
//        EV_INFO << "VEHICLE: processing group-key update" << endl;
//        processKeyUpdate(data);
//        delete msg;
//        return;
//    }
//
//    EV_INFO << "VEHICLE: unknown wireless message" << endl;
//    delete msg;
//}
//void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
//{
//    EV_INFO << "VEHICLE: onWSM() called" << endl;
//
//    auto *msg = dynamic_cast<TraCIDemo11pMessage *>(frame);
//    if (msg == nullptr) {
//        EV_WARN << "VEHICLE: Received frame is not TraCIDemo11pMessage!" << endl;
////        delete frame;
//        return;
//    }
//
//    const std::string data = msg->getDemoData();
//    EV_INFO << "VEHICLE: received packet: " << data << endl;
//
//    if (data.rfind("RESP|", 0) == 0) {
//        processRegistrationResponse(data);
////        delete msg; // Clean up consumed message
//        return;
//    }
//
//    if (data.rfind("UPDATE|", 0) == 0) {
//        processKeyUpdate(data);
////        delete msg; // Clean up consumed message
//        return;
//    }
//
////    delete msg;
//}
void GroupKeyVehicleApp::onWSM(BaseFrame1609_4 *frame)
{
    EV_INFO << "VEHICLE: onWSM() called" << endl;

    auto *msg = dynamic_cast<TraCIDemo11pMessage *>(frame);
    if (!msg) {
        EV_WARN << "VEHICLE: frame is not TraCIDemo11pMessage" << endl;
        // Do NOT delete frame here; DemoBaseApplLayer handles cleanup
        return;
    }

    const std::string data = msg->getDemoData();
    EV_INFO << "VEHICLE: received packet: " << data << endl;

    if (data.rfind("RESP|", 0) == 0) {
        EV_INFO << "VEHICLE: processing registration response" << endl;
        processRegistrationResponse(data);
        return; // NO delete msg!
    }

    if (data.rfind("UPDATE|", 0) == 0) {
        EV_INFO << "VEHICLE: processing group-key update" << endl;
//        updateStart = msg->getCreationTime();
//        updateAttempts++;
        processKeyUpdate(data);
//        updateSuccesses++;
//        updateLatency=simTime()-updateStart;
//        emit(updateLatencySignal,updateLatency);
//        recordScalar("updateLatency", updateLatency.dbl());
//        EV_INFO<<"updateLatency:"<<updateLatency.dbl()<<endl;
        return; // NO delete msg!
    }

    EV_INFO << "VEHICLE: unknown wireless message" << endl;
    // NO delete msg! DemoBaseApplLayer will delete 'frame' after onWSM returns.
}
void GroupKeyVehicleApp::processRegistrationResponse(const std::string& data)
{
    // record the start for cpu
    auto start = std::chrono::high_resolution_clock::now();
    std::stringstream ss(data);
    std::string type, id, versionString, x_i,w_i1,w_i2,w_i3,Astr;
    std::getline(ss, type, '|');
    std::getline(ss, id, '|');
    if (id != vehicleId)
            return;
    std::getline(ss, versionString, '|');
    std::getline(ss, x_i, '|');
    std::getline(ss,w_i1,'|');
    std::getline(ss,w_i2,'|');
    std::getline(ss,w_i3,'|');
    std::getline(ss,Astr,'|');
    deserializeFromHex(VSecrets.x_i,x_i);
    deserializeFromHex(VSecrets.w_i1,w_i1);
    deserializeFromHex(VSecrets.w_i2,w_i2);
    deserializeFromHex(VSecrets.w_i3,w_i3);
    deserializeFromHex(currentA,Astr);

    SGKDEngine::DeriveKey(VSecrets.w_i1, VSecrets.w_i2, VSecrets.w_i3, currentA, CGKey);
    // recorrd the end cpu time
    auto end = std::chrono::high_resolution_clock::now();
        // Calculate CPU duration in seconds
        double cpuTimeSec = std::chrono::duration<double>(end - start).count()*3;//multiplied by scale 3 to emulate the vehicle OBU CPU
     EV_INFO<<"Response Processing Latency="<<cpuTimeSec<<endl;
     recordScalar("responseProcessingLatency", cpuTimeSec);
    isregistered=true;
    int version = std::stoi(versionString);
    if (version >= currentKeyVersion) {
        currentKeyVersion = version;
        currentGroupKey = serializeToHex(CGKey);
        registrationSuccesses++;
        registrationLatency = simTime() - registrationStart+cpuTimeSec;
        emit(registrationLatencySignal, registrationLatency);
        EV_INFO << vehicleId << " registered with key version " << currentKeyVersion
                << ", latency=" << registrationLatency << "\n";
        EV_INFO << vehicleId << " derived key value " << currentGroupKey
                        << ", latency=" << registrationLatency << "\n";

    }
}

void GroupKeyVehicleApp::processKeyUpdate(const std::string& data)
{
    if(!isregistered)
    {
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    std::stringstream ss(data);
    std::string type, versionString, x_rstr, A_hatstr,updatestarttime;
    std::getline(ss, type, '|');
    std::getline(ss, versionString, '|');
    std::getline(ss, x_rstr, '|');
    std::getline(ss, A_hatstr,'|');
    std::getline(ss,updatestarttime,'|');
    updateStart=simtime_t(std::stod(updatestarttime));
    updateAttempts++;
    deserializeFromHex(currentA, A_hatstr);
    bn_t x_r;
    bn_null(x_r); bn_new(x_r);

    SGKDEngine::UpdateSecrets(x_r, currentA, VSecrets, CGKey);
    // recorrd the end cpu time
    auto end = std::chrono::high_resolution_clock::now();
        // Calculate CPU duration in seconds
        double cpuTimeSec = std::chrono::duration<double>(end - start).count()*3;
     EV_INFO<<"Update Processing Latency="<<cpuTimeSec<<endl;
     recordScalar("updateProcessingLatency", cpuTimeSec);
   updateSuccesses++;
   updateLatency=simTime()-updateStart+cpuTimeSec;
   emit(updateLatencySignal,updateLatency);
   recordScalar("updateLatency", updateLatency);
   EV_INFO<<"updateLatency:"<<updateLatency<<endl;

    int version = std::stoi(versionString);
    if (version > currentKeyVersion) {
        if (currentKeyVersion >= 0 && version > currentKeyVersion + 1)
            missedUpdates += version - currentKeyVersion - 1;

        currentKeyVersion = version;
        currentGroupKey = serializeToHex(CGKey);
        EV_INFO<<"Updated Key"<<currentGroupKey<<endl;
        keyUpdatesReceived++;
        emit(keyUpdateVersionSignal, currentKeyVersion);
    }
    else {
        staleUpdates++;
    }
    bn_clean(x_r);
}

void GroupKeyVehicleApp::finish()
{
    recordScalar("registrationAttempts", registrationAttempts);
    recordScalar("registrationSuccesses", registrationSuccesses);
    recordScalar("keyUpdatesReceived", keyUpdatesReceived);
    recordScalar("staleUpdates", staleUpdates);
    recordScalar("missedUpdates", missedUpdates);
    recordScalar("finalKeyVersion", currentKeyVersion);
    if (registrationSuccesses > 0)
        recordScalar("registrationLatency", registrationLatency);

    cancelAndDelete(registrationTimer);
    registrationTimer = nullptr;
    DemoBaseApplLayer::finish();
}

} // namespace groupkey
