#include "tcp_server.h"
using boost::asio::ip::tcp;

void sumfunc(int a, int b, tcp_connection::pointer sp);


tcp_server::tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), SERVER_PORT)), inf_pipeline()
{
    start_accept();
}

void tcp_server::start_accept()
{
    tcp_connection::pointer new_connection =
        tcp_connection::create(acceptor_.get_io_service(), this);

    acceptor_.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,
                   const boost::system::error_code &error)
{
    if (!error)
    {
        new_connection->start();
    }

    start_accept();
}

void tcp_server::enqueue_request(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){
    this->inf_pipeline.execute_serverless_function(client_id, model_id, model_input, sp);
}

// start the ioservice and the tcp server
int main()
{
	try
	{
		boost::asio::io_service io_service;
		tcp_server server(io_service);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
