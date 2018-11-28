#pragma once

#include "RdtReceiver.h"
#include "RdtSender.h"

#include <deque>


// Namespace for my implementation of GBN protocal
namespace GBNRdtProtocal {

    const int seqnum_ceil = 8;

    const int inital_seqnum = 0;
    const int initial_window_size = GBNRdtProtocal::seqnum_ceil / 2;

    // GBN receiver class
    class Receiver : public RdtReceiver {

        // window size agreed by both ends of the connection
        int window_size = GBNRdtProtocal::initial_window_size;
        int expected_seqnum;
        Packet last_ack_pkt;

    public:

        Receiver(unsigned window_size = GBNRdtProtocal::initial_window_size);
        virtual ~Receiver(void);

        virtual void receive(Packet &pkt);

    };


    // GBN sender class
    class Sender : public RdtSender {

        const bool using_fast_resend;
        int missed_ack_cnt = 0;

        // window size agreed by both ends of the connection
        int window_size = GBNRdtProtocal::initial_window_size;
        int window_base;            // currently earliest non-ACK-ed packet
        std::deque<Packet> window;  // sender window (empty on init)

    public:

        Sender(
            bool fast_resend = false,
            unsigned window_size = GBNRdtProtocal::initial_window_size
        );
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
