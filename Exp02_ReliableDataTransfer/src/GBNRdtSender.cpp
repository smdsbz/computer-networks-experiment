#include "stdafx.h"
#include "Global.h"

#include <stdexcept>
#include <cstring>  // memset, memcpy
#include <queue>

#include "GBNRdtProtocal.h"
using GBNRdtProtocal::Sender;


Sender::Sender(unsigned window_size) :
        window_size(window_size),
        window_base(GBNRdtProtocal::inital_seqnum) {
    if (window_size <= 0) {
        throw std::runtime_error("Sender window must have positive window"
            "size");
    }
    return;
}


Sender::~Sender(void) {
    return;
}


const int SENDER_TIMER_ID = 0;

bool Sender::send(Message &msg) {
    // don't send if reached right edge of window
    if (this->window.size() == this->window_size) {
        return false;
    }
    // send packet and save it in sender buffer
    this->window.push_back(Packet());   // make placeholder
    Packet &new_pkt = this->window.back();
    // NOTE: acknum not required thus ignored
    new_pkt.seqnum = (window_base + this->window.size() - 1) % this->window_size;
    // NOTE: require `sizeof(Message::data) == sizeof(Packet::payload)`
    std::memcpy(new_pkt.payload, msg.data, sizeof(msg.data));
    new_pkt.checksum = pUtils->calculateCheckSum(new_pkt);
    pUtils->printPacket("Sender sending new packet", new_pkt);
    pns->sendToNetworkLayer(RECEIVER, new_pkt);
    // start timer immediately for this packet
    pns->stopTimer(SENDER, SENDER_TIMER_ID);
    pns->startTimer(SENDER, Configuration::TIME_OUT, SENDER_TIMER_ID);
    return true;
}


void Sender::receive(Packet &pkt) {
    auto checksum = pUtils->calculateCheckSum(pkt);
    // legit ACK packet
    if (checksum == pkt.checksum) {
        // reassign window base loaction
        this->window_base = pkt.acknum;
        // do clean up
        while ((!this->window.empty())
                && (this->window.front().seqnum != this->window_base)) {
            this->window.pop_front();
        }
        // reset timer for window
        pns->stopTimer(SENDER, SENDER_TIMER_ID);
        if (!this->window.empty()) {
            pns->startTimer(SENDER, Configuration::TIME_OUT, SENDER_TIMER_ID);
        }
    }
    // corrupted packet
    else {
        pUtils->printPacket("Sender received corrupted packet", pkt);
        // do nothing about corrupted ACK packet
        /* pass */
    }
    return;
}


void Sender::timeoutHandler(int seqnum) {
    // NOTE: `seqnum` not used in GBN protocal thus ignored
    // resend all packet in window
    for (auto &each : this->window) {
        pUtils->printPacket("Sender re-sent previous packet due to timeout",
            each);
        pns->sendToNetworkLayer(RECEIVER, each);
    }
    pns->startTimer(SENDER, Configuration::TIME_OUT, SENDER_TIMER_ID);
    return;
}


bool Sender::getWaitingState(void) {
    return (this->window.size() == this->window_size);
}
