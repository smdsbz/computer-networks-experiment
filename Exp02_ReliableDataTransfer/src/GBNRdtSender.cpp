#include "stdafx.h"
#include "Global.h"

#include <stdexcept>
#include <cstring>  // memset, memcpy
#include <queue>

#include "GBNRdtProtocal.h"
#define RDX ( GBNRdtProtocal::seqnum_ceil )
using GBNRdtProtocal::Sender;


Sender::Sender(bool fast_resend, unsigned window_size) :
        using_fast_resend(fast_resend),
        window_size(window_size),
        window_base(GBNRdtProtocal::inital_seqnum) {
    if (window_size <= 0) {
        throw std::runtime_error("Sender window must have positive window"
            "size");
    }
    if (this->using_fast_resend) {
        std::cout << "Fast resend enabled!" << std::endl;
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
    new_pkt.seqnum = (this->window_base + this->window.size() - 1) % RDX;
    // NOTE: require `sizeof(Message::data) == sizeof(Packet::payload)`
    std::memcpy(new_pkt.payload, msg.data, sizeof(msg.data));
    new_pkt.checksum = pUtils->calculateCheckSum(new_pkt);
    // pUtils->printPacket("Sender sending new packet", new_pkt);
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
        /**** fast-resend plugin ****/
        if (pkt.acknum != (this->window_base + 1) % RDX) {
            ++this->missed_ack_cnt;
            if (this->missed_ack_cnt == 4) {
                this->missed_ack_cnt = 0;
                if (this->using_fast_resend && !this->window.empty()) {
                    // resend lefter-most packet in window
                    pns->stopTimer(SENDER, SENDER_TIMER_ID);
                    // pUtils->printPacket("Sender sending earliest packet in window due to fast-resend", this->window.front());
                    std::cout << "fast resend sign" << std::endl;
                    pns->sendToNetworkLayer(RECEIVER, this->window.front());
                    pns->startTimer(SENDER, Configuration::TIME_OUT, SENDER_TIMER_ID);
                }
            }
        }
        else {
            this->missed_ack_cnt = 0;
        }
        /**** end fast-resend plugin ****/
        // reassign window base loaction, i.e. ealiest sent but not yet ACK-ed packet
        // HACK: always have `acknum <= window_base + 1`
        this->window_base = pkt.acknum;
        this->print_window();
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
    // if corrupted packet, do nothing
    else {
        // pUtils->printPacket("Sender received corrupted packet", pkt);
        /* pass */
    }
    return;
}


void Sender::timeoutHandler(int seqnum) {
    // NOTE: `seqnum` not used in GBN protocal thus ignored
    // resend all packet in window
    for (auto &each : this->window) {
        // pUtils->printPacket("Sender re-sent previous packet due to timeout", each);
        pns->sendToNetworkLayer(RECEIVER, each);
    }
    pns->startTimer(SENDER, Configuration::TIME_OUT, SENDER_TIMER_ID);
    return;
}


bool Sender::getWaitingState(void) {
    return (this->window.size() == this->window_size);
}


void Sender::print_window(void) {
    std::cout << "Printing sender window..." << std::endl;
    if (this->window.empty()) {
        std::cout << "(Empty window)" << std::endl;
    }
    else {
        for (auto &each : this->window) {
            pUtils->printPacket("", each);
        }
    }
}
