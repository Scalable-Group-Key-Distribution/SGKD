package gkd_vanet.modules;

import org.car2x.veins.base.modules.BaseMobility;
import org.car2x.veins.modules.nic.Nic80211p;

module GroupKeyRSU
{
    parameters:
        @display("i=veins/sign/yellowdiamond;is=vs");

    gates:
        input veinsradioIn;
        input wiredIn;
        output wiredOut;

    submodules:
        appl: GroupKeyRSUApp {
            @display("p=60,50");
        }

        nic: Nic80211p {
            @display("p=60,166");
        }

        mobility: BaseMobility {
            @display("p=130,172;i=block/cogwheel");
        }

    connections:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;

        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;

        veinsradioIn --> nic.radioIn;

        wiredIn --> appl.wiredIn;
        appl.wiredOut --> wiredOut;
}
//package gkd_vanet.modules;
//
//import org.car2x.veins.base.modules.IBaseApplLayer;
//import org.car2x.veins.modules.nic.Nic80211p;
//import org.car2x.veins.base.modules.BaseMobility;
//
//module GroupKeyRSU
//{
//    parameters:
//        string applType = default("gkd_vanet.modules.GroupKeyRSUApp");
//        string nicType = default("Nic80211p");
//        @display("i=veins/sign/yellowdiamond;is=vs");
//
//    gates:
//        input veinsradioIn;
//        input wiredIn;
//        output wiredOut;
//
//    submodules:
//        appl: <applType> like IBaseApplLayer {
//            @display("p=60,50");
//        }
//
//        nic: <nicType> like org.car2x.veins.modules.nic.INic80211p {
//            @display("p=60,166");
//        }
//
//        mobility: BaseMobility {
//            @display("p=130,172;i=block/cogwheel");
//        }
//
//    connections:
//        nic.upperLayerOut --> appl.lowerLayerIn;
//        nic.upperLayerIn <-- appl.lowerLayerOut;
//        nic.upperControlOut --> appl.lowerControlIn;
//        nic.upperControlIn <-- appl.lowerControlOut;
//
//        veinsradioIn --> nic.radioIn;
//}
//package gkd_vanet.modules;
//
//import org.car2x.veins.nodes.RSU;
//
//module GroupKeyRSU extends RSU
//{
//    parameters:
//        applType = "gkd_vanet.modules.GroupKeyRSUApp";
//        @display("i=veins/sign/yellowdiamond;is=vs");
//
//    gates:
//        input wiredIn;
//        output wiredOut;
//
//    connections allowunconnected:
//        wiredIn --> appl.wiredIn;
//        appl.wiredOut --> wiredOut;
//}
//package gkd_vanet.modules;
//
//import org.car2x.veins.nodes.RSU;
//
//module GroupKeyRSU extends RSU
//{
//    parameters:
//        applType = "gkd_vanet.modules.GroupKeyRSUApp";
//        @display("i=veins/sign/yellowdiamond;is=vs");
//    gates:
//        input wiredIn;
//        output wiredOut;
//        input lowerLayerIn;
//        output lowerLayerOut;
//        input lowerControlIn;
//        output lowerControlOut;
//    connections allowunconnected:
//        wiredIn --> appl.wiredIn;
//        appl.wiredOut --> wiredOut;
//        
//}
