#include "stdafx.h"
#include "Global.h"

#include <stdexcept>
#include <cstring>  // memcpy, memset

#include "SRRdtProtocal.h"
using SRRdtProtocal::Receiver;


Receiver::Receiver(int window_size) :
        window_size(window_size),
        window_base(SRRdtProtocal::inital_seqnum),
        window(std::deque<Packet *>(this->window_size, 0)) {
    // default init last ACK packet
    last_ack_pkt.acknum = this->window_base - 1;
    last_ack_pkt.checksum = pUtils->calculateCheckSum(last_ack_pkt);
    return;
}


Receiver::~Receiver(void) {
    for (auto &each : this->window) {
        if (each != 0) {
            delete each;
            each = 0;
        }
    }
    return;
}


void Receiver::receive(Packet &pkt) {
    auto checksum = pUtils->calculateCheckSum(pkt);
    // corrupted packet
    if (checksum != pkt.checksum) {
        // pUtils->printPacket("Receivcer received corrupted packet", pkt);
        // re-send ACK packet
        // pUtils->printPacket("Receiver resending ACK packet", this->last_ack_pkt);
        pns->sendToNetworkLayer(SENDER, this->last_ack_pkt);
        return;
    }
    int diff = (pkt.seqnum - this->window_base + SRRdtProtocal::seqnum_ceil) % SRRdtProtocal::seqnum_ceil;
    // if in receive buffer window and correct order, deliver sequence of
    // packets to application layer
    if (diff == 0) {
        // pUtils->printPacket("Receiver received packet with correct order", pkt);
        // deliver first sequence of ACK-ed packets to application layer
        // TODO: ditch 0-th place in `this->window`
        this->window[0] = new Packet(pkt);
        Message msg;
        while (this->window.front() != 0) {
            std::memcpy(msg.data, this->window.front()->payload, sizeof(msg.data));
            pns->delivertoAppLayer(RECEIVER, msg);
            // right shift buffer window
            delete this->window.front();
            this->window.pop_front();
            this->window.push_back(0);
            this->window_base = (
                (this->window_base + 1)
                % SRRdtProtocal::seqnum_ceil
            );
            this->print_window();
        }
    }
    // if in receive window but out of order, buffer it
    else if (diff < this->window_size) {
        // pUtils->printPacket("Receiver received packet out of order", pkt);
        // // DEBUG: check if seqnum overlapped
        // if (this->window[diff] != 0) {
        //     throw std::runtime_error("SRRdtProtocal::Receiver::receive : "
        //         "packet seqnum overlapped!");
        // }
        this->window[diff] = new Packet(pkt);
    }
    // if not in current but previous receive window, send ACK as well
    else {
        // pUtils->printPacket("Receiver received packet of last window", pkt);
        /* pass */
    }
    // send ACK packet for this packet, either in current or previous
    // receive window
    this->last_ack_pkt.acknum = pkt.seqnum;
    this->last_ack_pkt.checksum = \
        pUtils->calculateCheckSum(this->last_ack_pkt);
    // pUtils->printPacket("Receiver sending ACK packet", this->last_ack_pkt);
    pns->sendToNetworkLayer(SENDER, this->last_ack_pkt);
    return;
}


void Receiver::print_window(void) {
    std::cout << "Printing sender window..." << std::endl;
    if (this->window.empty()) {
        std::cout << "(Empty window)" << std::endl;
    }
    else {
        for (auto &each : this->window) {
            if (each != 0) {
                pUtils->printPacket("", *each);
            }
            else {
                std::cout << "(packet chekced)" << std::endl;
            }
        }
    }
}
