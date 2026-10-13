package gkd_vanet.simulations;

import org.car2x.veins.nodes.Scenario;
import ned.DatarateChannel;
import gkd_vanet.modules.GroupKeyTA;
import gkd_vanet.modules.GroupKeyRSU;

network GroupKeyScenario extends Scenario
{
    parameters:
        int numRsu = default(10);
//        int numRSUs = default(10);
        //double roadLength @unit(m) = default(5000m);
        //double rsuRadius @unit(m) = default(300m);
        //double overlap @unit(m) = default(60m); // 10% overlap (540m effective spacing)
        @display("bgb=50000,500");

    submodules:
        ta: GroupKeyTA {
            @display("p=80,80;i=device/server");
        }

        rsu[numRsu]: GroupKeyRSU {
            @display("p=400,120;i=veins/sign/yellowdiamond;is=vs");
        }

    connections allowunconnected:
        for i=0..numRsu-1 {
            ta.wiredOut[i] --> DatarateChannel {
                datarate = ta.wiredDatarate;
                delay = ta.wiredDelay;
            } --> rsu[i].wiredIn;

            rsu[i].wiredOut --> DatarateChannel {
                datarate = ta.wiredDatarate;
                delay = ta.wiredDelay;
            } --> ta.wiredIn[i];
        }
        
}
//package gkd_vanet.simulations;
//
//import org.car2x.veins.nodes.Scenario;
//import org.car2x.veins.nodes.Car;
//import org.car2x.veins.nodes.RSU;
//import veins.base.connectionManager.ConnectionManager;
//import ned.DatarateChannel;
//
//network GroupKeyScenario extends Scenario
//{
//    parameters:
//        int numRsu = default(3);
//        @display("bgb=2500,500");
//    submodules:
//        ta: GroupKeyTA {
//            gates:
//                wiredIn[numRsu];
//                wiredOut[numRsu];
//            @display("p=80,80;i=device/server");
//        }
//        rsu[numRsu]: GroupKeyRSU {
//            @display("p=400+700*index,120;i=veins/sign/yellowdiamond;is=vs");
//        }
//    connections allowunconnected:
//        for i=0..numRsu-1 {
//            ta.wiredOut[i] --> DatarateChannel {
//                datarate = ta.wiredDatarate;
//                delay = ta.wiredDelay;
//            } --> rsu[i].wiredIn;
//            rsu[i].wiredOut --> DatarateChannel {
//                datarate = ta.wiredDatarate;
//                delay = ta.wiredDelay;
//            } --> ta.wiredIn[i];
//        }
//}
