#include "request_defs.h"

nw_request::nw_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){
    this->client_id = client_id;
    this->model_id = model_id;
    this->model_input = model_input;
    this->connection_sp = sp;
}

pr_request::pr_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){
    this->client_id = client_id;
    this->model_id = model_id;
    this->model_input = model_input;
    this->connection_sp = sp;
}

ld_request::ld_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){
    this->client_id = client_id;
    this->model_id = model_id;
    this->model_input = model_input;
    this->connection_sp = sp;
}
// This class stores the state of each model on the server
model_state::model_state(int cpu_prep_time, int gpu_memcopy_time, int gpu_exec_time){
    this->cpu_prep_time = cpu_prep_time;
    this->gpu_memcopy_time = gpu_memcopy_time;
    this->gpu_exec_time = gpu_exec_time;
    // set state to A intially
    this->current_state = model_state::A;
}