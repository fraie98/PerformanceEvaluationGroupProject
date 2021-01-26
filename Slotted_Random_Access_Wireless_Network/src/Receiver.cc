//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Receiver.h"

Define_Module(Receiver);

void Receiver::initialize()
{
    responseTimeSignal_ = registerSignal("responseTimeSignal");
    numPacketReceivedSignal_ = registerSignal("numPacketReceivedSignal");
}


void Receiver::handleMessage(cMessage *msg)
{
    PacketMsg* pkt = check_and_cast<PacketMsg*>(msg);
    EV << "Receiver " << this->getId() << ":packet received from transmitter " << pkt->getIdTransmitter() << endl;
    //Register received signal
    emit(numPacketReceivedSignal_, 1);

    handleResponseTime(pkt);
    delete(msg);
}

void Receiver::handleResponseTime(PacketMsg* pkt){
    simtime_t respTime = simTime() - pkt->getCreationTime();
    EV << "Receiver " << this->getId() << ": response Time = " << respTime << endl;
    emit(responseTimeSignal_, respTime.dbl());
}


/**
 * To handle the threshold parameter will be emitted a '0'
 * if the threshold is respected, '1' if not respected
 *//*
void Receiver::handleThreshold(PacketMsg* pkt){
    simtime_t threshold = par("timeThreshold");
    simtime_t respTime = simTime() - pkt->getCreationTime();
    int emit_value;


    if(respTime < threshold){
        EV << "Receiver " << this->getId() << ": threshold(" << threshold << "s) respected" << endl;
        emit_value = 0;
    } else {
        EV << "Receiver " << this->getId() << ":threshold(" << threshold << "s) not respected" << endl;
        emit_value = 1;
    }
    emit(thresholdSignal_, emit_value);
}
*/
