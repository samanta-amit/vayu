#ifndef INFERENCE_PIPELINE_H
#define INFERENCE_PIPELINE_H

#include <vector>
#include <map>
#include <string>
#include "request_defs.h"
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

class inference_pipeline {
    private:
        // 3 queues
        // q1 < nw req > named nw_queue
        // q2 < initialized req> init_queue
        // q3 < unloaded req> unloaded_queue

        // synchronizaton stuff for the queues
        // 3 mutexes and 3 condition variables

        // map for loaded models with random values for now
        std::map<int, model_state> srv_state;
        
        // The request pipeline (queues along with synchronization stuff)
        // the request from the network is first enqueued here.
        std::deque<nw_request> nw_queue;
        std::mutex nw_queue_mutex;
        std::condition_variable nw_queue_cv;
        
        // processed request queue from the above step
        std::deque<pr_request> pr_queue;
        std::mutex pr_queue_mutex;
        std::condition_variable pr_queue_cv;
        
        // processed request queue for the last step wich will be making the 
        // inference and returning the result to the client asynchronously
        // which I don't know how to do.
        std::deque<ld_request> ld_queue;
        std::mutex ld_queue_mutex;
        std::condition_variable ld_queue_cv;

        std::thread nw_req_thread;
        std::thread pr_req_thread;
        std::thread ld_req_thread;
        
    public:
        inference_pipeline();
        void make_inference(int client_id, int model_id, std::string model_input, tcp_connection::pointer);
        void echo_input_back(int a, int b, std::string string_op, tcp_connection::pointer sp);
        // functions that are needed to be performed at each step of the pipeline.
        // each funcition runs in a separate thread (or threads) to pickup request 
        // from the queue before it in the pipeline and puts it in the queue next
        // to it in the pipleline.
        void process_nw_request(void);
        void process_pr_request(void);
        void process_ld_request(void);
         
};

#endif