#include<stdio.h>
#include<string.h>
#include<omnetpp.h>

using namespace omnetpp;

class Toc: public cSimpleModule
{
private:
    int counter = 0;         /* Counter to receive the packets and send the acknowledgment when it is equal to the window size. */
    int seq_count = 0;       /* Variable to keep track of the latest sequence number received successfully. */
    int seq_count_copy = 0;  /* Keep a copy of the latest sequence number received successfully, to check if the packet is dropped later or not. */
    int window = 0;          /* Variable to store the window size and send it to tic. */
    int init_msg_flag = 0;   /* Variable to check if the message is an initial message from tic. */
    int lost_pkt_flag = 0;   /* Variable to check if there is a packet loss. */
    int repeat_seq_flag = 0; /* Set the flag if the sequence count has reached to 255. */
    int loop = 3;            /* Variable to send the acknowledgment. */
    int lost_pkt_seq_no = 0; /* Variable to track if the packet lost for the first time is getting lost again. */
protected:
    virtual void handleMessage(cMessage *msg) override;
    virtual void generateMessage();
    virtual void receivedMessage(cMessage *msg);
    virtual void sendMessage(cMessage *msg);
};

Define_Module(Toc);

/*
 * This function is called when there is a packet received from tic.
 * It calls the appropriate function depending on the packet being received or lost.
 */
void Toc:: handleMessage(cMessage *msg) {

    if (uniform(0, 1) < 0.1) {
        EV << "Losing message\n";
        if ((!lost_pkt_seq_no) || (lost_pkt_seq_no == msg->par("seq_no").longValue())) {
            EV<<"send that the packet is lost";
            bubble("message lost");
            lost_pkt_flag = 1;
            generateMessage();
            lost_pkt_seq_no = msg->par("seq_no").longValue();
        }
        delete msg;
    } else {
        receivedMessage(msg);
        generateMessage();
    }
}

/*
 * This function is used to generate acknowledgment packet based on
 *      a. The packet is received successfully.
 *      b. The packet is lost.
 */
void Toc:: generateMessage()
{
    char msgname[20];
    cMessage *msg;

    /* Generating Initial message, and sending window size */
    if(!init_msg_flag) {
        strcpy(msgname, "Init_toc_msg");
        init_msg_flag = 1;

        msg = new cMessage(msgname);
        msg->addPar("ack_no");
        msg->addPar("win_size");
        msg->par("ack_no").setLongValue(seq_count);
        msg->par("win_size").setLongValue(window);
        send(msg, "out");
    } else { /* Generating messages with Acknowledgment number*/


        if (lost_pkt_flag) {
            EV <<"send that the packet is lost in generateMessage\n";
            sendMessage(msg);
            lost_pkt_flag = 0;
            counter = 0;
        }
        else {

            if (lost_pkt_seq_no && (lost_pkt_seq_no == seq_count_copy)) {
                EV <<"receiving the correct sequence number\n";
                lost_pkt_seq_no = 0;
                lost_pkt_flag = 0;
            }

            if (!lost_pkt_seq_no) {
            if (counter <= loop) {
                EV << "Inside generating ack no counter <= window\n";
                EV << "'Lost pkt flag = " << lost_pkt_flag << "\n";
                EV<<"'value of counter at start of loop = " << counter << "\n";
                counter++;

                if ((counter < loop) && (seq_count_copy == (seq_count + 1))) { /*receiving the packet for first time, in right sequence*/
                    EV << "receiving the packet for first time\n";
                    //lost_pkt_seq_no = 0;
                    if(seq_count_copy == 255) {
                        EV << " sequence is greater than 255\n";
                        repeat_seq_flag = 1;
                    }
                    seq_count = seq_count + 1;
                }
                else if ((counter == loop) && (seq_count_copy == (seq_count + 1))) { /*receiving the last packet according to the window size*/
                        EV << "receiving the last packet\n";

                        counter = 0;
                        seq_count = seq_count + 1;
                        EV <<"'value of counter = " << counter << "\n";
                        sendMessage(msg);
                        if (seq_count_copy == 255) {
                            EV<< "sequence is greater than 255\n";
                            repeat_seq_flag = 1;
                        }

                    }
                }

            }
        }
    }
}

/*
 * This function is used to send the packet out of toc
 */
void Toc:: sendMessage(cMessage *msg) {

    char msgname[20];

    strcpy(msgname, "Message from toc");
    msg = new cMessage(msgname);
    msg->addPar("ack_no");
    msg->addPar("pkt_lost");
    msg->par("ack_no").setLongValue(seq_count);
    msg->par("pkt_lost").setLongValue(lost_pkt_flag);
    send(msg, "out");
}

/*
 *This function is called by handleMessage().
 *It does the following task:
 *      a. Retrive the message received.
 *      b. Print the message logs.
 *      c. Store the useful information from the message.
 *      d. Delete the message if it is not required.
 */
void Toc:: receivedMessage(cMessage *msg)
{
    if(msg->par("seq_no").longValue() == 0) { /* Initial Message received from tic, requesting for window size */
        EV << "'Message received from: " << msg->getName() << "\n";
        EV << "'Data received: \n" << msg->par("seq_no").longValue() << "\n";
        /* Store the window size in variable "window" */
        window = par("win_size");
        delete msg;
    } else { /* Started receiving the packets */
        EV << "'Message received from: " << msg->getName() << "\n";
        EV << "'sequence received: \n" << msg->par("seq_no").longValue() << "\n";
        /* Store the sequence number received in the message */
        seq_count_copy = msg->par("seq_no").longValue();
        EV <<"'repeat_seq_flag: " << repeat_seq_flag << "\n";
        if (seq_count_copy == 1 && repeat_seq_flag) { /* Start receiving the packets from 1 after the packet limit reached to 255 */
            repeat_seq_flag = 0;
            seq_count = 0;
        }
        delete msg;
    }
}
