#include "rpc_client.h"
#include <thread>
// Create a bunch of clients and each client makes an rpc call on a seperate thread

int main(int argc, char* argv[]){
    
    std::vector<std::thread> client_threads;
    std::vector<rpc_client*> rpc_clients;
    std::string server_addr = "0.0.0.0";
    std::string port = "5005";
    // two clients in two threads
    for (int i = 0; i < 2; i++)    
    {
        rpc_client* cl = new rpc_client(i, server_addr, port);
        rpc_clients.push_back( cl );
        // cl->call("make_inference")
        // client_threads.push_back(std::thread(&rpc_client::call, cl, "make_inference"));
        client_threads.push_back(std::thread(&rpc_client::loop_call, cl, "make_inference"));
    }
    for (auto &thread : client_threads)
    {
        thread.join();
    }
    return 0;
    // Create a thread using member function
}