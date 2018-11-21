#include "stdafx.h"
#include "Global.h"

#include <stdexcept>
#include <cstring>

#include "SRRdtProtocal.h"
using SRRdtProtocal::Sender;


Sender::Sender(int window_size) :
        window_size(window_size),
        window_base(SRRdtProtocal::inital_seqnum),
        last_pkt_seqnum(
            (SRRdtProtocal::inital_seqnum - 1 + SRRdtProtocal::seqnum_ceil)
            % SRRdtProtocal::seqnum_ceil
        ),
        window(std::deque<Packet *>(this->window_size, 0)) {
    return;
}


Sender::~Sender(void) {
    for (auto &each : this->window) {
        if (each != 0) {
            delete each;
            each = 0;
        }
    }
    return;
}


void Sender::printStat(void) {
    std::cout << "Sender stats:\n"
        << "\twindow_base = " << this->window_base << "\n"
        << "\tlast_pkt_seqnum = " << this->last_pkt_seqnum << std::endl;
    return;
}


bool Sender::send(Message &msg) {
    // refuse if window is full
    if ((this->last_pkt_seqnum - this->window_base + 1 + SRRdtProtocal::seqnum_ceil) % SRRdtProtocal::seqnum_ceil
            == this->window_size) {
        return false;
    }
    // else, send and save it for future re-sends
    this->last_pkt_seqnum = (
        (this->last_pkt_seqnum + 1 + SRRdtProtocal::seqnum_ceil)
            % SRRdtProtocal::seqnum_ceil
    );
    Packet *ptr_pkt = new Packet();
    ptr_pkt->seqnum = this->last_pkt_seqnum;
    std::memcpy(ptr_pkt->payload, msg.data, sizeof(ptr_pkt->payload));
    ptr_pkt->checksum = pUtils->calculateCheckSum(*ptr_pkt);
    this->window[
        (this->last_pkt_seqnum - this->window_base + SRRdtProtocal::seqnum_ceil)
            % SRRdtProtocal::seqnum_ceil
    ] = ptr_pkt;
    // send to remote receiver
    pUtils->printPacket("Sender sending new packet", *ptr_pkt);
    pns->sendToNetworkLayer(RECEIVER, *ptr_pkt);
    this->printStat();
    // start timer
    pns->startTimer(SENDER, Configuration::TIME_OUT, ptr_pkt->seqnum);
    return true;
}


void Sender::receive(Packet &pkt) {
    auto checksum = pUtils->calculateCheckSum(pkt);
    // ignore corrupted ACK packet
    if (checksum != pkt.checksum) {
        pUtils->printPacket("Sender received corrupted ACK packet", pkt);
        return;
    }
    // ignore out-ranged ACK packet
    int diff = (pkt.acknum - this->window_base + SRRdtProtocal::seqnum_ceil) % SRRdtProtocal::seqnum_ceil;
    if (diff >= this->window_size) {
        pUtils->printPacket("Sender received ACK packet out of window", pkt);
        this->printStat();
        return;
    }
    pUtils->printPacket("Sender received valid ACK packet", pkt);
    // mark packet as ACK-ed
    Packet * &target_cell = this->window[diff];
    delete target_cell;
    target_cell = 0;
    // stop timer for this packet
    pns->stopTimer(SENDER, pkt.acknum);
    // if ACK-ing window base, do buffer clean up
    std::cout << "before cleanup" << std::endl;
    this->printStat();
    int range = (this->last_pkt_seqnum - this->window_base + SRRdtProtocal::seqnum_ceil) % SRRdtProtocal::seqnum_ceil;
    std::cout << "cleanup range = " << range << std::endl;
    if (range >= this->window_size) {
        throw std::runtime_error("range calculation error");
    }
    for (; range != 0 && this->window.front() == 0; --range) {
        this->window.pop_front();
        this->window.push_back(0);
        this->window_base = (this->window_base + 1) % SRRdtProtocal::seqnum_ceil;
    }
    std::cout << "after cleanup" << std::endl;
    this->printStat();
    return;
}


void Sender::timeoutHandler(int seqnum) {
    // re-send this packet
    Packet &pkt = *this->window[
        (seqnum - this->window_base + SRRdtProtocal::seqnum_ceil)
            % SRRdtProtocal::seqnum_ceil
    ];
    pUtils->printPacket("Sender re-sending packet", pkt);
    pns->sendToNetworkLayer(RECEIVER, pkt);
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
    return;
}


bool Sender::getWaitingState(void) {
    return (
        (this->last_pkt_seqnum - this->window_base + 1 + SRRdtProtocal::seqnum_ceil)
            % SRRdtProtocal::seqnum_ceil
        == this->window_size
    );
}
