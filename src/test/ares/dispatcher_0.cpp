// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/buffer.hpp"
#include "ares/cmdline_arg_parser.hpp"
#include "ares/dispatcher.hpp"
#include "ares/net_tk.hpp"
#include "ares/random.hpp"
#include "ares/server_interface.hpp"
#include "ares/socket.hpp"
#include "ares/socket_acceptor.hpp"
#include "ares/string_util.hpp"
#include "ares/thread.hpp"
#include <assert.h>
#include <memory>

using namespace std;
using namespace ares;

namespace
{
enum { READER_CHUNK_SIZE = 100*1024 };

string listen_port = "27462";       // test server's listen port
int num_bytes = 100*1024*1024;      // total amount to send (all sessions)
int num_sessions = 50;              // number of sessions
int num_bytes_per_session = 0;      // amount to send per session
bool random_order = true;           // send data in random order?
int num_bytes_per_send = 0;         // send chunk size (<= 0 means random)

vector<vector<Byte> > send_data;    // N vectors of M random bytes
vector<vector<Byte> > recv_data;    // should be == send_data
vector<int> send_pos;               // indicates how much has been sent


// Print usage instructions to stdout, then exit the program.
void display_usage()
{
    printf("\n"
           "dispatcher_0: Stress-test the ares dispatcher component.\n"
           "\n"
           "You can control how dispatcher_0 runs by entering the command\n"
           "followed by various arguments. To specify parameters, you use\n"
           "keywords (NOT case sensitive):\n"
           "\n"
           "    Format: dispatcher_0 KEYWORD=value (KEYWORD=value ...)\n"
           "    Example: dispatcher_0 NSESSIONS=20 NBYTES=1048576\n"
           "\n"
           "Keyword         Description (Default)\n"
           "------------------------------------------------------------\n"
           "HELP            if 'Y', displays this message and exits (N)\n"
           "PORT            port to use for test server (27462)\n"
           "NSESSIONS       number of sessions (50)\n"
           "NBYTES          total number of bytes to send (100MB)\n"
           "NBYTES_PER_SEND bytes to send at a time (random)\n"
           "RANDOM_ORDER    send to sessions in random order? (Y)\n"
           "\n");
    exit(0);
}

// Returns a n-byte vector containing random data.
vector<Byte> generate_random_bytes(int n)
{
    vector<Byte> v(n);
    for (int i = 0; i < n; i++)
        v[i] = Byte(Random::next_int(256));
    return v;
}
}

// Reader's job is to connect to the test server and read up to
// send_data[id].size() bytes of data over the network; when the expected
// number of bytes have been read, the Reader compares what it read to what
// the server allegedly sent, and raises an error if they do not match.
class Reader : public Thread::Runnable {
  public:
    Reader(int id)
            : m_id(id)
            , m_thread(this)
            , m_was_successful(false)
    {
        m_thread.start();
    }

    virtual ~Reader() {}

    void run() try
    {
        vector<Byte>& received_bytes = recv_data[m_id];
        vector<Byte>& sent_bytes = send_data[m_id];

        // Connect to the test server.
        printf("reader(%d): connecting to %s:%s\n", m_id,
               net_tk::my_hostname().c_str(), listen_port.c_str());
        auto_ptr<Socket> socket(connect_tcp(net_tk::my_hostname(),
                                            listen_port));

        // Read the expected data from our socket.
        Byte input[READER_CHUNK_SIZE];
        while (received_bytes.size() < sent_bytes.size()) {
            int n = socket->read(input, sizeof(input));
            if (n < 0) {
                printf("reader(%d): encountered end-of-file\n", m_id);
                break;
            }
            else if (n > 0) {
                received_bytes.insert(received_bytes.end(), input, input + n);
            }
            printf("reader(%d): read %d bytes (total is %d of %d)\n",
                   m_id, n, int(received_bytes.size()), int(sent_bytes.size()));
        }

        // Display a summary message upon completion.
        if (received_bytes == sent_bytes) {
            printf("reader(%d): OK: done reading\n", m_id);
            m_was_successful = true;
        }
        else {
            printf("reader(%d): ERROR: done reading, data sent/recv mismatch\n"
                   "reader(%d): (sent %d bytes, received %d bytes)\n",
                   m_id, m_id, int(sent_bytes.size()),
                   int(received_bytes.size()));
        }
    }
    catch (ares::Exception& e) {
        fprintf(stderr, "\nreader(%d): ERROR at %s:%d\n  in %s:\n%s\n",
                m_id, __FILE__, __LINE__, __PRETTY_FUNCTION__,
                e.to_string().c_str());
        throw;
    }

    bool is_running() const { return m_thread.is_running(); }
    bool was_successful() const { return m_was_successful; }

  private:
    int const m_id;
    Thread m_thread;
    bool m_was_successful;
};

class Test_session : public Session_rep {
  public:
    Test_session(Server_interface& server, Socket* socket, int id)
            : Session_rep(server, socket), m_id(id)
    {
        use_slave_process_for_output(true); // use dispatcher
    }

    bool do_handle_input(ares::Buffer&) { return false; }

  private:
    int const m_id;
};

class Test_server : public Server_interface {
  public:
    Test_server() : m_dispatcher(*this) { m_dispatcher.startup(); }
    virtual ~Test_server() { m_dispatcher.shutdown(); }
    void add_session(Session) {}
    void remove_session(Session) {}
    void enqueue_command(Command*) {}
    void enqueue_delayed_command(Command*, int) {}
    void dispatch(Session s, Buffer* b) { m_dispatcher.dispatch(s, b); }
    job::Scheduler& scheduler() { return m_scheduler; }
    void shutdown() {}
    Date started() const { return Date(); }
    int uptime() const { return 0; }

    void gather_and_display_stats()
    {
        Dispatcher_statistics s = m_dispatcher.statistics();
        printf("dispatcher: elapsed_sec: %d\n",
               s.elapsed_sec());
        printf("dispatcher: writes: %d\n",
               s.writes());
        printf("dispatcher: writes_per_sec: %.2f\n",
               s.writes_per_sec());
        printf("dispatcher: zero_writes: %d\n",
               s.zero_writes());
        printf("dispatcher: zero_writes_per_sec: %.2f\n",
               s.zero_writes_per_sec());
        printf("dispatcher: bytes_sent: %d\n",
               s.bytes_sent());
        printf("dispatcher: bytes_sent_per_sec: %.2f\n",
               s.bytes_sent_per_sec());
        printf("dispatcher: bytes_per_write: %d\n",
               s.bytes_per_write());
        printf("dispatcher: buffers_added: %d\n",
               s.buffers_added());
        printf("dispatcher: buffers_added_per_sec: %.2f\n",
               s.buffers_added_per_sec());
        printf("dispatcher: buffers_sent: %d\n",
               s.buffers_sent());
        printf("dispatcher: buffers_sent_per_sec: %.2f\n",
               s.buffers_sent_per_sec());
    }

  private:
    Dispatcher m_dispatcher;
    job::Scheduler m_scheduler;
};

int main(int argc, char** argv) try
{
    Cmdline_arg_parser args(argc, argv);

    // Display help message if requested.
    if (args.exists("help"))
        if (boost::to_lower_copy(args.get_string("help")) != "n")
            display_usage();

    // Process command-line arguments.
    if (args.exists("port"))
        listen_port = args.get_string("port");
    if (args.exists("nsessions"))
        num_sessions = args.get_int("nsessions");
    if (args.exists("nbytes"))
        num_bytes = args.get_int("nbytes");
    if (args.exists("nbytes_per_send")) {
        if (boost::to_lower_copy(args.get_string("nbytes_per_send"))=="random")
            num_bytes_per_send = 0;
        else
            num_bytes_per_send = args.get_int("nbytes_per_send");
    }
    if (args.exists("random_order"))
        random_order =
                boost::to_lower_copy(args.get_string("random_order")) != "n";

    num_bytes_per_session = num_bytes / num_sessions;

    // Display a summary of how we've been configured to run.
    printf("main: test server will listen on port %s\n", listen_port.c_str());
    printf("main: spawning %d sessions\n", num_sessions);
    printf("main: sending %d bytes (%d per session)\n",
           num_bytes, num_bytes_per_session);

    if (num_bytes_per_send <= 0)
        printf("main: sending data in randomly sized chunks\n");
    else
        printf("main: sending data in %d-byte chunks\n", num_bytes_per_send);

    if (random_order)
        printf("main: sending to sessions in random order\n");
    else
        printf("main: sending to sessions in round-robin order\n");

    // Generate random data to send over the network.
    Random::seed(current_time());
    printf("main: generating %d bytes of random data for %d sessions\n",
           num_bytes_per_session, num_sessions);
    for (int i = 0; i < num_sessions; i++)
        send_data.push_back(generate_random_bytes(num_bytes_per_session));
    recv_data.resize(num_sessions);

    // Create a test server object.
    printf("main: creating test server\n");
    Test_server server;

    // Create a listen socket.
    printf("main: binding listen socket to %s:%s\n",
           net_tk::my_hostname().c_str(), listen_port.c_str());
    Socket_acceptor acceptor;
    acceptor.bind(net_tk::my_hostname(), listen_port);

    // Start the reader threads and accept their connections (ONE AT A TIME!).
    vector<Reader*> readers;
    vector<Session> sessions;
    printf("main: creating %d readers\n", num_sessions);
    for (int i = 0; i < num_sessions; i++) {
        vector<Socket*> sockets;

        printf("main: creating a reader (%d left)\n", num_sessions - i - 1);
        readers.push_back(new Reader(i));
        printf("main: waiting for the reader to connect\n");
        if (acceptor.wait_for_connection(3000, sockets) <= 0) {
            fprintf(stderr, "main: FATAL: expected a connection!\n");
            exit(2);
        }
        assert(sockets.size() == 1);
        Socket* s = sockets[0];
        printf("main: connection from %s:%d, creating session %d\n",
               s->remote_address().c_str(), s->remote_port(), i);
        sessions.push_back(new Test_session(server, s, i));
    }

    // Send data to the sessions.
    send_pos.assign(num_sessions, 0);
    if (random_order) {
        // Send data to the sessions in random order.
        printf("main: sending randomly generated data over local network\n");
        int num_sessions_left_with_data = num_sessions;
        while (num_sessions_left_with_data > 0) {
            // Choose a random session that still has unsent data.
            int session_num = 0;
            do {
                session_num = Random::next_int(num_sessions);
            } while (send_pos[session_num] >= num_bytes_per_session);

            int pos = send_pos[session_num];
            int remaining = num_bytes_per_session - pos;
            int num_bytes_to_send = (num_bytes_per_send > 0)
                                    ? min(remaining, num_bytes_per_send)
                                    : 1 + Random::next_int(remaining);

            // Send a dispatch.
            sessions[session_num]->send(Buffer(&send_data[session_num][pos],
                                               num_bytes_to_send));
            printf("main: session %d sent %d of %d\n",
                   session_num, num_bytes_to_send, remaining);
            send_pos[session_num] += num_bytes_to_send;

            if (send_pos[session_num] == num_bytes_per_session) {
                num_sessions_left_with_data--;
                printf("main: session %d sent its data (%d sessions left)\n",
                       session_num, num_sessions_left_with_data);
            }
        }
    }
    else {
        int num_sends;
        do {
            num_sends = 0;
            for (int i = 0; i < num_sessions; i++) {
                int pos = send_pos[i];
                int remaining = num_bytes_per_session - pos;
                if (remaining <= 0)
                    continue;
                int num_bytes_to_send = (num_bytes_per_send > 0)
                                        ? min(remaining, num_bytes_per_send)
                                        : 1 + Random::next_int(remaining);

                sessions[i]->send(Buffer(&send_data[i][pos],
                                         num_bytes_to_send));
                printf("main: session %d sent %d of %d\n",
                       i, num_bytes_to_send, remaining);
                send_pos[i] += num_bytes_to_send;
                ++num_sends;
            }
        } while (num_sends > 0);
    }

    // Confirm that all readers exited successfully.
    for (int num_tries = 0; num_tries < 100; num_tries++) {
        printf("main: pausing for a brief period\n");
        milli_sleep(500);
        bool success = true;
        for (int i = 0; i < num_sessions; i++) {
            // If a reader hasn't finished yet, abort this check and retry.
            if (readers[i]->is_running()) {
                printf("main: reader %d is still running\n", i);
                success = false;
                break;
            }

            // If a reader exited unsuccessfully, the test failed.
            if (!readers[i]->was_successful()) {
                printf("main: ERROR: data mismatch for reader %d\n"
                       "main: (sent %d bytes, received %d bytes)\n",
                       i, int(send_data[i].size()), int(recv_data[i].size()));

                // Print some debugging info about what went wrong.
                for (int j = 0; j < num_sessions; j++)
                    if (recv_data[i] == send_data[j])
                        printf("main: OOPS! %d == %d\n", i, j);

                for_each(readers.begin(), readers.end(), delete_fun<Reader>);
                exit(2);  // test failed
            }
        }

        if (success) {
            printf("main: OK: data sent and data received match\n");
            for_each(readers.begin(), readers.end(), delete_fun<Reader>);
            server.gather_and_display_stats();
            return 0;  // test succeeded
        }
    }

    // If we got here, the test failed.
    printf("main: ERROR: giving up; one or more readers won't exit\n");
    for_each(readers.begin(), readers.end(), delete_fun<Reader>);
    exit(2); // test failed
}
catch (ares::Exception& e) {
    fprintf(stderr, "\nERROR at %s:%d\n  in %s:\n%s\n",
            __FILE__, __LINE__, __PRETTY_FUNCTION__,
            e.to_string().c_str());
}
