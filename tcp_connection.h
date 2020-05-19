
#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <msgpack.hpp> //Depends on at least v2 of https://github.com/msgpack/msgpack-c
using boost::asio::ip::tcp;
using namespace std;
// forward declare the server
class tcp_server;

class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;
	static boost::shared_ptr<tcp_connection> create(boost::asio::io_service& io_service, tcp_server* this_server);
	tcp::socket& socket();
	void start();
	tcp_server *this_server;

private:
	tcp::socket socket_;
	boost::asio::streambuf input_buffer_;
	class msgpack::unpacker unp;

	tcp_connection(boost::asio::io_service& io_service, tcp_server* this_server);

	void handle_write(const boost::system::error_code& /*error*/,
			size_t /*bytes_transferred*/);
	
	void handle_read(const boost::system::error_code& ec);
	void dispatch(const msgpack::object &deserialized);


};
#endif