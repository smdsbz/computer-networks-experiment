#include "stdafx.h"
#include "Global.h"

#include <stdexcept>
#include <cstring>  // memcpy, memset

#include "GBNRdtProtocal.h"
using GBNRdtProtocal::Receiver;
#define RDX ( GBNRdtProtocal::seqnum_ceil )


Receiver::Receiver(unsigned window_size) :
        window_size(window_size),
        expected_seqnum(GBNRdtProtocal::inital_seqnum) {
    // should be 1 less than inital expected_seqnum
    this->last_ack_pkt.acknum = this->expected_seqnum - 1;
    // this->last_ack_pkt.seqnum = 0;  // NOTE: index of ACK packet is not required
    this->last_ack_pkt.checksum = \
        pUtils->calculateCheckSum(this->last_ack_pkt);
    return;
}


Receiver::~Receiver(void) {
    return;
}


void Receiver::receive(Packet &pkt) {
    auto checksum = pUtils->calculateCheckSum(pkt);
    if ((checksum == pkt.checksum) && (pkt.seqnum == this->expected_seqnum)) {
        // print debug message
        pUtils->printPacket("Receiver received expected packet", pkt);
        // make message and deliver to application layer
        Message msg;    // content to be delivered
        std::memcpy(msg.data, pkt.payload, sizeof(pkt.payload));
        pns->delivertoAppLayer(RECEIVER, msg);
        // reply ACK with index of next expected packet
        this->expected_seqnum = (this->expected_seqnum + 1) % RDX;
        this->last_ack_pkt.acknum = this->expected_seqnum;  // ask for next packet
        this->last_ack_pkt.checksum = pUtils->calculateCheckSum(this->last_ack_pkt);
        pUtils->printPacket("Receiver sending ACK packet", this->last_ack_pkt);
        pns->sendToNetworkLayer(SENDER, this->last_ack_pkt);
    }
    // if packet corrupted or out of order, re-send ACK for expected packet
    else {
        if (checksum != pkt.checksum) {
            pUtils->printPacket("Receiver received corrupted packet", pkt);
        }
        else if (pkt.seqnum != this->expected_seqnum) {
            pUtils->printPacket("Receiver received unexpected packet", pkt);
            std::cout << "Receiver is expecting ACK packet for seqnum = "
                << this->expected_seqnum << std::endl;
        }
        else {
            throw std::runtime_error("[DEBUG] GBNRdtReceiver::receive : "
                "unrecognized condition!");
        }
        // resend last ACK packet
        pUtils->printPacket("Receiver re-sending ACK packet", this->last_ack_pkt);
        pns->sendToNetworkLayer(SENDER, this->last_ack_pkt);
    }
    return;
}
