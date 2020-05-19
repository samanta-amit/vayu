#ifndef REQUEST_DEFS_H
#define REQUEST_DEFS_H
#include <string>
#include "tcp_connection.h"
class nw_request{
    public:
    int client_id;
    int model_id;
    std::string model_input;
    // shared pointer to the connection object
    // important to keep this alive as long as the request is in the pipeline
    tcp_connection::pointer connection_sp;
    nw_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp);
};

/*
    TODO add all the required fields to implement:
    a) Actual inference (link to executables, model weights etc)
    b) Fields needed for scheduling to the two classes below

    One of the ways could be to link these as a library to the so of the 
    tvm and other dependencies that we want to invoke.
*/
class pr_request{
    public:
    int client_id;
    int model_id;
    std::string model_input;
    tcp_connection::pointer connection_sp;
    pr_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp);

};

class ld_request{
    public:
    int client_id;
    int model_id;
    std::string model_input;
    tcp_connection::pointer connection_sp;
    ld_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp);
};

class model_state{
    //std::string model_name;
    // wait times in miliseconds for stubs
    public:
    int cpu_prep_time;
    int gpu_memcopy_time;
    int gpu_exec_time;
    // Name the states
    enum _state{
        A, // uninitialized
        B, // initialized not loaded
        C  // Loaded    
    };  

    // track the state of each model
    enum _state current_state;
    model_state(int cpu_prep_time, int gpu_memcopy_time, int gpu_exec_time);
};

#endif