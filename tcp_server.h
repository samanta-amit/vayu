#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <string>
#include <thread>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "tcp_connection.h"
#include "inference_pipeline.h"

using boost::asio::ip::tcp;
using namespace std;


class tcp_server
{
public:
	tcp_server(boost::asio::io_service& io_service);
	// rpc parameters received from the client
	void enqueue_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp);

private:
	inference_pipeline inf_pipeline;
    const uint32_t SERVER_PORT = 5005;
	tcp::acceptor acceptor_;
    void start_accept();

	void handle_accept(tcp_connection::pointer new_connection,
			const boost::system::error_code& error);
};

#endif