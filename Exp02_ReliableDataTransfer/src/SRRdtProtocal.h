#pragma once

#include "RdtReceiver.h"
#include "RdtSender.h"

#include <deque>


// Namespace for my implementation of SR protocal
namespace SRRdtProtocal {

    const int inital_seqnum = 0;
    const int seqnum_ceil = 4 << 1;

    // SR receiver class
    class Receiver : public RdtReceiver {

        const int window_size = SRRdtProtocal::seqnum_ceil >> 1;
        int window_base;
        Packet last_ack_pkt;
        std::deque<Packet *> window;

    public:

        Receiver(int window_size = SRRdtProtocal::seqnum_ceil >> 1);
        virtual ~Receiver(void);

        virtual void receive(Packet &pkt);

        virtual void print_window(void);
    };


    // SR sender class
    class Sender : public RdtSender {

        const int window_size = SRRdtProtocal::seqnum_ceil >> 1;
        int window_base;        // currently earliest non-ACK-ed packet
        int last_pkt_seqnum;    // seqnum of last sent packet in window
        std::deque<Packet *> window;    // sender window (empty on init)

        void printStat(void);

    public:

        Sender(int window_size = SRRdtProtocal::seqnum_ceil >> 1);
        virtual ~Sender(void);
        // if not busy, send and return `true`; else return `false`
        virtual bool send(Message &msg);
        // receive ACK from remote receiver
        virtual void receive(Packet &pkt);
        virtual void timeoutHandler(int seqnum);
        virtual bool getWaitingState(void);

        virtual void print_window(void);
    };

};
