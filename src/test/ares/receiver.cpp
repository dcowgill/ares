// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/buffer.hpp"
#include "ares/net_tk.hpp"
#include "ares/random.hpp"
#include "ares/socket.hpp"
#include "ares/socket_acceptor.hpp"
#include "ares/receiver.hpp"
#include "ares/thread.hpp"
#include <assert.h>

using namespace std;
using namespace ares;

namespace
{
string const LISTEN_SERVICE = "27462";

enum {
    TOTAL_DATA_SIZE = 100*1024*1024,
    NUM_SESSIONS = 100,
    SEND_DATA_SIZE = TOTAL_DATA_SIZE/NUM_SESSIONS
};

vector<vector<Byte> > send_data;    // N vectors of M random bytes
vector<vector<Byte> > recv_data;    // should be == send_data
}

class Test_session : public Session_rep {
  public:
    Test_session(Server_interface& server, Socket* socket, int id)
            : Session_rep(server, socket)
            , m_id(id)
    {
        printf("(debug: constructing session %d)\n", id);
    }

    bool do_handle_input(Buffer& b)
    {
        recv_data[m_id].insert(recv_data[m_id].end(), b.begin(), b.end());
        b.clear();
        return true;
    }

  private:
    int const m_id;
};

class Test_server : public Server_interface {
  public:
    virtual ~Test_server() {}
    void add_session(Session) {}
    void remove_session(Session) {}
    void enqueue_command(Command*) {}
    void enqueue_delayed_command(Command*, int) {}
    void dispatch(Session, Buffer*) {}
    job::Scheduler& scheduler() { return m_scheduler; }
    void shutdown() {}
    Date started() const { return Date(); }
    int uptime() const { return 0; }

  private:
    job::Scheduler m_scheduler;
};

class Listener : public Thread::Runnable {
  public:
    Listener(Server_interface& s, Receiver& r)
            : m_server_interface(s)
            , m_receiver(r)
    {}

    void run() try
    {
        enum { WAIT_SEC=3 };

        printf("lsnr: entering listener run() function\n");
        Socket_acceptor acceptor;
        printf("lsnr: binding to %s:%s\n", net_tk::my_hostname().c_str(),
               LISTEN_SERVICE.c_str());
        acceptor.bind(net_tk::my_hostname(), LISTEN_SERVICE);
        int connections_remaining = NUM_SESSIONS;
        while (connections_remaining > 0) {
            printf("lsnr: waiting up to %ds for a connection (%d left)\n",
                   WAIT_SEC, connections_remaining);
            vector<Socket*> sockets;
            if (acceptor.wait_for_connection(WAIT_SEC*1000, sockets) <= 0) {
                fprintf(stderr, "lsnr: FATAL: expected a connection!\n");
                exit(2);
            }
            for (int i = 0; i < int(sockets.size()); i++) {
                Socket* socket = sockets[i];
                printf("lsnr: accepted connection from %s:%d\n",
                       socket->remote_address().c_str(),
                       socket->remote_port());
                m_receiver.add_session(
                    new Test_session(m_server_interface,
                                     socket,
                                     NUM_SESSIONS-connections_remaining));
                connections_remaining--;
            }
        }
        printf("lsnr: exiting listener run() function\n");
    }
    catch (ares::Exception& e) {
        fprintf(stderr, "\nERROR at %s:%d\n  in %s:\n%s\n",
                __FILE__, __LINE__, __PRETTY_FUNCTION__,
                e.to_string().c_str());
    }

  private:
    Server_interface& m_server_interface;
    Receiver& m_receiver;
    bool m_stopped;
};

int main(int argc, char** argv) try
{
    Random::seed(current_time());

    //
    // Generate random data to send over the network.
    //
    printf("main: generating %d bytes of random data for %d sessions\n",
           SEND_DATA_SIZE, NUM_SESSIONS);
    vector<Byte> v(SEND_DATA_SIZE);
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SEND_DATA_SIZE; j++) {
            v[j] = Byte(Random::next_int(256));
        }
        send_data.push_back(v);
    }
    recv_data.resize(NUM_SESSIONS);
    assert(send_data.size() == recv_data.size());

    //
    // Start the receiver and listener threads
    //
    Test_server server;
    Receiver receiver(server);
    printf("main: starting independent receiver thread\n");
    receiver.startup();
    printf("main: creating listener thread\n");
    Thread(new Listener(server, receiver)).start();
    printf("main: sleeping for 200 ms\n");
    milli_sleep(200);  // gives the listener thread a chance to startup

    //
    // Establish NUM_SESSIONS connections to the listener
    //
    vector<Socket*> sockets;
    for (int i = 0; i < NUM_SESSIONS; i++) {
        printf("main: connecting to %s:%s\n",
               net_tk::my_hostname().c_str(),
               LISTEN_SERVICE.c_str());
        Socket* s = connect_tcp(net_tk::my_hostname(), LISTEN_SERVICE);
        sockets.push_back(s);
    }

    //
    // Make sure receiver has the right number of sessions
    //
    printf("main: sleeping for 200 ms\n");
    milli_sleep(200);
    int num_sessions = receiver.num_sessions();
    printf("main: receiver now has %d session(s)\n", num_sessions);
    assert(num_sessions == NUM_SESSIONS);

    //
    // Send data over the sockets in an unpredictable way
    //
    printf("main: sending randomly generated data over socket\n");
    int sessions_left_with_data = NUM_SESSIONS;
    vector<int> send_pos(NUM_SESSIONS, 0);
    while (sessions_left_with_data > 0) {
        // Choose a random session to start sending.
        int session_num = 0;
        for (;;) {
            session_num = Random::next_int(NUM_SESSIONS);
            if (send_pos[session_num] < SEND_DATA_SIZE)
                break;
        }

        int pos = send_pos[session_num];
        int remaining = SEND_DATA_SIZE-pos;

        // Pick a random amount ([1, remaining]) of data to send.
        int send_amount = 1+Random::next_int(remaining);

        // Write to the socket
        Socket* s = sockets[session_num];
        int n = s->write(&send_data[session_num][pos], send_amount);
        printf("main: session %d wrote %d/%d\n", session_num, n, remaining);
        send_pos[session_num] += n;

        if (send_pos[session_num] == SEND_DATA_SIZE) {
            sessions_left_with_data--;
            printf("main: session %d sent all its data (%d sessions left)\n",
                   session_num, sessions_left_with_data);
        }
    }

    //
    // Confirm that data sent == data received; allow up to 10 failures, with
    // 100ms sleeps between them, before giving up
    //
    for (int n = 0; n < 10; n++) {
        int const sleep_millis = 100;
        printf("main: sleeping for %d ms\n", sleep_millis);
        milli_sleep(sleep_millis);
        bool success = true;
        for (int i = 0; i < NUM_SESSIONS; i++) {
            if (send_data[i] != recv_data[i]) {
                printf("main: data sent/recv (session %d) mismatch\n",i);
                printf("main: (sent %d bytes, received %d bytes)\n",
                       int(send_data[i].size()), int(recv_data[i].size()));
                printf("main: checking again...\n");
                success = false;
                break;
            }
        }
        if (success) {
            receiver.shutdown();
            for_each(sockets.begin(), sockets.end(), delete_fun<Socket>);
            printf("main: OK: data sent and data received match\n");
            return 0;  // success
        }
    }

    //
    // If we got here, the test failed
    //
    receiver.shutdown();
    for_each(sockets.begin(), sockets.end(), delete_fun<Socket>);
    printf("main: ERROR: giving up; data sent/recv do not match\n");
    return 2; // failure
}
catch (ares::Exception& e) {
    fprintf(stderr, "\nERROR at %s:%d\n  in %s:\n%s\n",
            __FILE__, __LINE__, __PRETTY_FUNCTION__,
            e.to_string().c_str());
}
