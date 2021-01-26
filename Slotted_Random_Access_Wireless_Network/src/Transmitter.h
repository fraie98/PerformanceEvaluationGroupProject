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

#ifndef __SLOTTED_RANDOM_ACCESS_WIRELESS_NETWORK_TRANSMITTER_H_
#define __SLOTTED_RANDOM_ACCESS_WIRELESS_NETWORK_TRANSMITTER_H_

#include <omnetpp.h>
#include <queue>
#include "PacketMsg_m.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Transmitter : public cSimpleModule
{
private:
    simsignal_t numPacketCreatedSignal_;
    simsignal_t numPacketOnBufferSignal_;
    simsignal_t meanPacketSignal_;
    double meanPktInBuffer_;

    std::queue<PacketMsg*> buffer;

    int maxBackoffTime, backoffTime;

    int numChannels;

    simtime_t meanInterarrivalTime;
    double sendProbability;

protected:
    virtual void initialize();
    virtual void scheduleNextPacket();
    virtual void handleMessage(cMessage *msg);
    virtual void handleArrivedPacket(cMessage *msg);
    virtual void handleChannelPacket(cMessage* msg);
    virtual void computeModuleStatistics();
    virtual void finish();
};

#endif
