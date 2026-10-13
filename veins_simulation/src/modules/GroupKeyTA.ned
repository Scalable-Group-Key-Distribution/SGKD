package gkd_vanet.modules;

simple GroupKeyTA
{
    parameters:
        @class(gkd_vanet::GroupKeyTA);

        int numRsu = default(10);
        double wiredDatarate @unit(bps) = default(100Mbps);
        double wiredDelay @unit(s) = default(1ms);
        double keyUpdateInterval @unit(s) = default(3s);

        @display("i=device/server");

    gates:
        input wiredIn[numRsu];
        output wiredOut[numRsu];
        
}
//package gkd_vanet.modules;
//
//simple GroupKeyTA
//{
//    parameters:
//        @class(gkd_vanet::GroupKeyTA);
//        int numRsu = default(3);
//        double wiredDatarate @unit(bps) = default(100Mbps);
//        double wiredDelay @unit(s) = default(1ms);
//        double keyUpdateInterval @unit(s) = default(30s);
//        @display("i=device/server");
//    gates:
//        input wiredIn[numRsu];
//        output wiredOut[numRsu];
//        input lowerLayerIn;
//        output lowerLayerOut;
//        input lowerControlIn;
//        output lowerControlOut;
//}
