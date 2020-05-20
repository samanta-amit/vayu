#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include "serverless_pipeline.h"

serverless_pipeline::serverless_pipeline() : 
    // the steps that initially are cheap and are assumed to be given
    // can be performed here for example linking so and other stuff.
    // initialize all mutexes
    nw_queue_mutex(),
    nw_queue_cv(),
    pr_queue_mutex(),
    pr_queue_cv(),
    ld_queue_mutex(),
    ld_queue_cv(),
    // spin up all the pipeline threads.
    nw_req_thread(&serverless_pipeline::process_nw_request, this),
    pr_req_thread(&serverless_pipeline::process_pr_request, this),
    ld_req_thread(&serverless_pipeline::process_ld_request, this){
    
    // random values for the models
    // model_state(cpu_wait_time,gpu_mem_time,gpu_time)
    srv_state.insert(std::make_pair(1, model_state(1,300,100)));
    srv_state.insert(std::make_pair(2, model_state(1,500,150)));
    srv_state.insert(std::make_pair(3, model_state(1,900,200)));
}

void serverless_pipeline::execute_serverless_function(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){
    // request from the client first enters the pipeline from here.
    // acquire the nw_queue_mutex to put the request on the queue and
    // signal the nw_queue_cv to wake up the next thread in the pipeline
    // to execute the request. the next thread in the pipeline is the 
    // initialize model thread which processes the request from the network
    // (links the so and loads the weights in the ram) 
    // waiting on the nw_queue_cv.
    
    // this also locks the mutex
    std::unique_lock<std::mutex> nw_mutex_mgr(nw_queue_mutex);
    std::cout<<"inside make_nference, acquiring the lock now "<<std::endl;
    // create the input_request type object that will be pushed to the nw_queue

    
    auto nr = nw_request(client_id, model_id, model_input, sp);
    // TODO: Implement part of scheduling logic. (where to put the request)
    // TODO: additional optimization to skip steps if the model is already intialized
    nw_queue.push_back(nr);
    nw_mutex_mgr.unlock();
    nw_queue_cv.notify_one();
    // notify/wakeup a model_initializer thread.
   
}

void serverless_pipeline::process_nw_request(void){
    // first step of the processing pipeline
    while(1){

        std::unique_lock<std::mutex> nw_mutex_mgr(nw_queue_mutex);
        nw_queue_cv.wait(nw_mutex_mgr, [this](){ return nw_queue.size() != 0; });
        // TODO: implementation of step A from discussion for processing.
        // pop a request from the nw_queue and process the request.
        // Steps to process the request:
        // intially we can assume that the tvm libs/shared objects are already loaded in the memory.
        // Model weights are assumed to be in the memory as well...?
        // So this method does nothing for now?
        auto nr = nw_queue.back();
        // pop the request
        nw_queue.pop_back();
        // Unlock after print to sync prints too
        nw_mutex_mgr.unlock();

        // can unlock the nw_queue_mutex here as we are mostly done
        std::cout<<"inside the process network request thread processing request from client "<<nr.client_id<<std::endl;
        // does what it says

        // sleep for cpu_wait_time
        auto x = srv_state.find(nr.model_id);
        std::this_thread::sleep_for(std::chrono::milliseconds( (x->second).cpu_prep_time ));
        // enqueue the request for the next thread in the pipline 
        std::unique_lock<std::mutex> pr_mutex_mgr(pr_queue_mutex);
        pr_request pr = pr_request(nr.client_id, nr.model_id, nr.model_input, nr.connection_sp);
        pr_queue.push_back(pr);
        pr_mutex_mgr.unlock();
        // signal the cv of the next task in the pipeline
        pr_queue_cv.notify_one();

    }
}
void serverless_pipeline::process_pr_request(void){
    while(1){
        std::unique_lock<std::mutex> pr_mutex_mgr(pr_queue_mutex);
        pr_queue_cv.wait(pr_mutex_mgr, [this](){return pr_queue.size() != 0; });

        auto pr = pr_queue.back();
        pr_queue.pop_back();
        pr_mutex_mgr.unlock();
        // TODO: implementation of step B from discussion.
        // Steps for processing:
        // Transfer weights from memory to gpu
        // std::cout<<" inside the pr_queue thread processing request from nw_queue"<<pr.client_id<<std::endl;
        // enqueue the request for the next thread in the pipline
        auto x = srv_state.find(pr.model_id);
        std::this_thread::sleep_for(std::chrono::milliseconds( (x->second).gpu_memcopy_time )); 
        std::unique_lock<std::mutex> ld_mutex_mgr(ld_queue_mutex);
        ld_request lr = ld_request(pr.client_id, pr.model_id, pr.model_input, pr.connection_sp);
        ld_queue.push_back(lr);
        ld_mutex_mgr.unlock();
        ld_queue_cv.notify_one();
    }
    return;    
}

void serverless_pipeline::process_ld_request(void){
    while(1){
        std::unique_lock<std::mutex> ld_mutex_mgr(ld_queue_mutex);
        ld_queue_cv.wait(ld_mutex_mgr, [this](){return ld_queue.size() != 0; });
        // TODO: implementation of step C from discussion.
        // Invoke the function to make the inference
        auto lr = ld_queue.back();
        ld_queue.pop_back();
        ld_mutex_mgr.unlock();
        auto x = srv_state.find(lr.model_id);
        std::this_thread::sleep_for(std::chrono::milliseconds( (x->second).gpu_memcopy_time )); 
        echo_input_back(lr.client_id, lr.model_id, lr.model_input, lr.connection_sp);

        // Finally return the output back to the client
        // std::cout<<" inside the ld_queue thread processing request from pr_queue"<<lr.client_id<<std::endl;

    }
    return;
}


// Write the input string back to the socket for stubs. With actual implementation
// the output of the inference will be written to the socket.
void serverless_pipeline::echo_input_back(int client_id, int model_id, std::string op_string, tcp_connection::pointer sp){
    msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	packer.pack_array(2);
    int64_t response_type = 1;
    packer.pack_int(response_type); //Responses have type 1, this will be assigned some semantics in the future... hopefully
    packer.pack_str(op_string.size());
    packer.pack_str_body(op_string.c_str(), op_string.size());
    msgpack::object_handle result;
    msgpack::unpack(result, sbuf.data(), sbuf.size());
    
    std::cout<<result.get()<<std::endl;   
	//Send response (synchronous, for simplicity)
	string outBuff(sbuf.data(), sbuf.size());
	cout<<"echoeing input string back "<<endl;
    boost::asio::write(sp->socket(), boost::asio::buffer(outBuff));

}
