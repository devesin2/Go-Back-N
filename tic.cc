#include<stdio.h>
#include<string.h>
#include<omnetpp.h>

using namespace omnetpp;

class Tic: public cSimpleModule
{
private:
    simtime_t timeout;
    cMessage *timeoutEvent;  /* To trigger the timeout event */
    int counter = 3;
    int init_msg_flag = 0;   /* Flag to send the initial control message to toc */
    int lost_pkt_flag = 0;   /* Flag to track down if the packet sent to toc is lost or not */
    int seq_count = 0;       /* To track the sequence count of the packets */
    int win_size = 0;        /* Variable to store the window size sent from toc */
    int i = 0;               /* Variable to create multiple objects to send multiple packets to toc */
    int rollover = 0;        /* Variable to reset the packets once the sequence count has reached to 255 */
public:
    Tic();
    virtual ~Tic();
protected:
    virtual void generateMessage();
    virtual void sendMessage(cMessage *msg);
    virtual void receivedMessage(cMessage *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};


Define_Module(Tic);

Tic::Tic()
{
    timeoutEvent = nullptr;
}

Tic::~Tic()
{
    cancelAndDelete(timeoutEvent);
}

void Tic:: initialize() {
    timeout = 0.5;
    timeoutEvent = new cMessage("timeoutEvent");
    generateMessage();
    EV << "start timer\n";
    scheduleAt(simTime()+timeout, timeoutEvent);
}

/*
 * 1. This function is called when
 *      a. There is a self message(i.e., When a timeout event occurs).
 *      b. When there is an acknowledgment packet from toc
 * 2. It calls the appropriate function according to the packet received by it.
 * 3. It schedules the timeout event once a packet is sent out of tic.
 */
void Tic:: handleMessage(cMessage *msg) {

        if(msg == timeoutEvent) {
            if(counter) {
                counter --;
                EV<< "Timeout expired, re-sending message\n";
                if (seq_count == 0) { /* What if the initial message is dropped, re-send the initial message again */
                    init_msg_flag = 0;
                    generateMessage();
                } else {
                    seq_count = seq_count - win_size;
                    generateMessage();
                  }
                scheduleAt(simTime()+timeout, timeoutEvent);
            } else {
                EV << "No response from toc, Exiting the program\n";
            }
        } else {
            receivedMessage(msg);
            cancelEvent(timeoutEvent);
            counter = 3;
            generateMessage();
            scheduleAt(simTime()+timeout, timeoutEvent);
        }
}

/*
 * Generate an initial message or a new Message to send to toc.
 */
void Tic::generateMessage()
{
    char msgname[20];

    if(!init_msg_flag) {   /* sending initial message, to get the window size */
        init_msg_flag = 1;
        strcpy(msgname, "init_packet");
        cMessage *msg = new cMessage(msgname);
        msg->addPar("seq_no");
        msg->par("seq_no").setLongValue(seq_count);
        sendMessage(msg);
    }
    else {  /* Start with packet transmission, window size is set by now */

        strcpy(msgname, "Packet from tic");

        /* To send multiple packets to toc */
        for( ; i < win_size; i++) {

            cMessage *msg[win_size];
            seq_count = seq_count + 1;

            msg[i] = new cMessage(msgname);
            msg[i]->addPar("seq_no");
            msg[i]->par("seq_no").setLongValue(seq_count);

            sendMessage(msg[i]);

            if (seq_count >= 255) {
                seq_count = 0;
            }
        }
   }
    return;
}

/* Send the message to toc */
void Tic::sendMessage(cMessage *msg) {
        send(msg, "out");
}

/*
 * 1. Retrieve the message
 * 2. Print the logs on console
 * 3. Provide the necessary data to send the new message
 */
void Tic::receivedMessage(cMessage *msg) {

    if(msg->par("ack_no").longValue() == 0) { /* execute this when there is an initial message response from toc */
        /*
         * 1. Check the window size variable of the message
         * 2. Store the window size in win_size
         * 3. Delete the message
         */
        EV << "'Message received from: " << msg->getName() << "\n" << msg->par("ack_no").longValue() << "\n";
        EV << "'window size: " << msg->par("win_size").longValue() << "\n";
        win_size = msg->par("win_size").longValue();
        delete msg;
    } else { /* execute this once we start receiving the acknowledgment from toc */
        EV << "'Message received from: " << msg->getName() << "\n" << msg->par("ack_no").longValue() << "\n";
        lost_pkt_flag = msg->par("pkt_lost").longValue();
        if (lost_pkt_flag) { /* Set the value of 'i' if there is a packet loss */
            EV<<"lost_pkt_flag is high\n";
            i = 0;
            EV <<"'value of i = " << i << "\n";
            seq_count = msg->par("ack_no").longValue();
        } else {
            /* Set the value of 'i' when the limit is reached to 255 */
            if (seq_count < msg->par("ack_no").longValue()) {
                rollover = 255 + seq_count;
                i = rollover - msg->par("ack_no").longValue();
            }
            else { /* set the value of 'i' if there is no packet loss */
            i = seq_count - msg->par("ack_no").longValue();
            EV <<"'value of i = " << i << "\n";
            }
        }
        if (seq_count >= 255) {
            seq_count = 0;
        }
        delete msg;
    }
}

