package gkd_vanet.modules;

import org.car2x.veins.base.modules.IBaseApplLayer;

simple GroupKeyVehicleApp like IBaseApplLayer
{
    parameters:
        @class(gkd_vanet::GroupKeyVehicleApp);

        int headerLength = default(88bit) @unit(bit);

        bool sendBeacons = default(false);
        int beaconLengthBits = default(256bit) @unit(bit);
        int beaconUserPriority = default(7);
        double beaconInterval = default(1s) @unit(s);
        bool avoidBeaconSynchronization = default(true);

        int dataLengthBits = default(1024bit) @unit(bit);
        bool dataOnSch = default(false);
        int dataUserPriority = default(7);

        bool sendWSA = default(false);
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval = default(1s) @unit(s);

        @display("i=device/car");

    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
        
}
//package gkd_vanet.modules;
//
//import org.car2x.veins.base.modules.IBaseApplLayer;
//
//simple GroupKeyVehicleApp like IBaseApplLayer
//{
//    parameters:
//        @class(gkd_vanet::GroupKeyVehicleApp);
//
//        int headerLength = default(88bit) @unit(bit);
//
//        bool sendBeacons = default(false);
//        int beaconLengthBits = default(256bit) @unit(bit);
//        int beaconUserPriority = default(7);
//        double beaconInterval = default(1s) @unit(s);
//        bool avoidBeaconSynchronization = default(true);
//
//        int dataLengthBits = default(1024bit) @unit(bit);
//        bool dataOnSch = default(false);
//        int dataUserPriority = default(7);
//
//        bool sendWSA = default(false);
//        int wsaLengthBits = default(250bit) @unit(bit);
//        double wsaInterval = default(1s) @unit(s);
//
//        @display("i=device/car");
//}
//package gkd_vanet.modules;
//
//simple GroupKeyVehicleApp like org.car2x.veins.base.modules.IBaseApplLayer
//{
//    parameters:
//        @class(gkd_vanet::GroupKeyVehicleApp);
//         int headerLength = default(88bit) @unit(bit);
//
//        bool sendBeacons = default(false);
//        int beaconLengthBits = default(256bit) @unit(bit);
//        int beaconUserPriority = default(7);
//        double beaconInterval = default(1s) @unit(s);
//        bool avoidBeaconSynchronization = default(true);
//
//        int dataLengthBits = default(1024bit) @unit(bit);
//        bool dataOnSch = default(false);
//        int dataUserPriority = default(7);
//
//        bool sendWSA = default(false);
//        int wsaLengthBits = default(250bit) @unit(bit);
//        double wsaInterval = default(1s) @unit(s);
//
//        @display("i=device/car");
//      gates:
//          input lowerLayerIn;
//        output lowerLayerOut;
//        input lowerControlIn;
//        output lowerControlOut;
//        
//        
//        
//}
