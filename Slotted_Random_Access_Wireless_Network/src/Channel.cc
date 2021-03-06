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

#include "Channel.h"
#include "PacketMsg_m.h"

Define_Module(Channel);

void Channel::initialize()
{
    throughputSignal_ = registerSignal("throughputSignal");
    collisionSignal_ = registerSignal("collisionSignal");
    wholeResponseTimeSignal_ = registerSignal("wholeResponseTimeSignal");

    // Array Initialization
    int numOfChannels = getAncestorPar("numChannels");

    //Handler for degeneracy test
    if(numOfChannels==0)
        endSimulation();

    for(int i=0; i<numOfChannels; i++)
        isCollided_.push_back(false);

    scheduleTimeSlot();

}

void Channel::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        EV << "Received self message" << endl;
        // Delete the trigger message
        delete(msg);

        /* If it's a self message then it means that it is
         * the trigger of the start of the slot.
         * So I have to control all the packet in
         * order to understand in which channels there
         * are collisions*/
        findCollisions();

        /* Now the channel transmit to the receivers the ACK for the
         * packets that don't collide and send a NACK
         * for the packets that collide.
         * It transmits to the receiver the non-collided packets */
        transmission();



        // Schedule the next time slot
        scheduleTimeSlot();
    }
    else
    {
        /* If it's not a self message then it means that
         * this is a packet.
         * So I have to push it in the vector of packets
         * for this slot*/
        PacketMsg* newPkt = check_and_cast<PacketMsg*>(msg);
        newPkt->setIdGate(newPkt->getArrivalGate()->getIndex());
        EV << "CH: Packet from tx " << newPkt->getIdTransmitter() << " arrived at Sub-Channel" << newPkt->getIdChannel() << endl;
        packetsOfSlot_.push_back(newPkt);
    }
}


/* Control every packet and set isCollided[i] to true
 * if there is a collision in the channel i*/
void Channel::findCollisions()
{
    std::vector<int> n_collisions(isCollided_.size(), 0);

    for(int i = 0; i<(int)packetsOfSlot_.size(); i++)
    {
        int ch = packetsOfSlot_[i]->getIdChannel();

        if(n_collisions[ch] == 1)
        {
            isCollided_[ch] = true;
            emit(collisionSignal_, 1);
        }
        n_collisions[ch]++;
    }
}

/* Transmit ACK or NACK to the transmitters
 * and non-collided packets to the receivers*/
void Channel::transmission()
{
    int numTx = getAncestorPar("numTransmitters");
    std::vector<bool> triggeredTx(numTx, false);
    int packetSent = 0;

    for(int i = 0; i<(int)packetsOfSlot_.size(); i++)
    {

        int idTx = packetsOfSlot_[i]->getIdTransmitter();
        int idGate = packetsOfSlot_[i]->getIdGate();

        // this transmitter doesn't need to be triggered with a TRIGGER message
        triggeredTx[idGate] = true;

        if(isCollided_[packetsOfSlot_[i]->getIdChannel()])
        {
            // A collision occur for this packet
            // send a NACK to the transmitter
            EV << "CH: NACK sended to Transmitter " << idTx<< endl;
            cMessage* nack = new cMessage("NACK");
            send(nack,"out_tx",idGate);

            delete(packetsOfSlot_[i]);
        }
        else
        {
            // No collision
            // Send a ACK to the transmitter
            EV << "CH: ACK sended to Transmitter " << idTx<< endl;
            cMessage* ack = new cMessage("ACK");
            send(ack,"out_tx",idGate);

            //Send the pkt to the receiver
            //Reused indexTx to get the id of the port
            EV << "Sended packet to Receiver " << idGate<< endl;
            send(packetsOfSlot_[i],"out_rx",idGate);

            //Utility: computation of response time
            simtime_t respTime = simTime() - packetsOfSlot_[i]->getCreationTime();
            EV << "CH: response Time = " << respTime << endl;
            emit(wholeResponseTimeSignal_, respTime.dbl());

            //Count packet sent to the receiver
            packetSent++;
        }
    }

    // Emit throughput (number of packets successfully sent in the current time-slot)
    EV << "CH: sent " << packetSent << " packets" << endl;

    if(isCollided_.size() > 0)
        emit(throughputSignal_, (long)(packetSent));
    else
        emit(throughputSignal_, 0);



    /* Trigger the tx which don't transmit a packet in the previous timeslot.
     * So the ones that are not been triggered yet*/
    triggerOthers(triggeredTx);


    // Reset vectors
    /* The following instruction it's equivalent
     * to packetsOfSlot_.clear() but force the
     * reallocation of the vector. This ensures that
     * the vector capacity will change (to 0)*/
    packetsOfSlot_.clear();

    std::vector<PacketMsg*>().swap(packetsOfSlot_);

    for(int i = 0; i<(int)isCollided_.size(); i++)
        isCollided_[i] = false;
}

/* It triggers the transmitters that have not been triggered yet */
void Channel::triggerOthers(std::vector<bool> triggeredTx)
{
    for(int i = 0; i < (int)triggeredTx.size(); i++)
    {
        /*
         * check which transmitter needs to be triggered
         */
        if(triggeredTx[i] == false)
        {
            cMessage* trigger = new cMessage("TRIGGER");
            EV << "CH: Trigger sent to tx at gate" << i << endl;
            send(trigger,"out_tx",i);
        }
    }
}


/* Send a self message in order to trigger the start
 * of the time slot */
void Channel::scheduleTimeSlot()
{
    simtime_t timeSlot = par("timeSlotSize");
    cMessage* timeSlotTrigger = new cMessage("timeSlotTrigger");

    /*Schedule the sending*/
    scheduleAt(simTime()+timeSlot,timeSlotTrigger);
}


/**
 * Delete every packet left in the Channel
 */
void Channel::finish(){
    for(PacketMsg* p: packetsOfSlot_){
        delete(p);
    }
    packetsOfSlot_.clear();
    std::vector<PacketMsg*>().swap(packetsOfSlot_);
}




