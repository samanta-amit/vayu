#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class rpc_client{
    private:
    int client_id;
    boost::asio::io_service io_service;
    tcp::resolver resolver;
    tcp::resolver::query query;
    tcp::resolver::iterator endpoint_iterator;
    tcp::socket socket;
    public:
    rpc_client(int client_id, std::string server_addr, std::string port);
    void call(std::string call_string);
    void loop_call(std::string call_string);

};



#endif