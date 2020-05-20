#include <iostream>
#include <string>
#include <thread>
#include <boost/bind.hpp>
#include "tcp_connection.h"
#include "tcp_server.h"
using boost::asio::ip::tcp;
using namespace std;

boost::shared_ptr<tcp_connection> tcp_connection::create(boost::asio::io_service& io_service, tcp_server* this_server)
{
    return pointer(new tcp_connection(io_service, this_server));
}

tcp::socket& tcp_connection::socket()
{
    return socket_;
}

void tcp_connection::start()
{
    boost::asio::async_read(socket_, input_buffer_,
        boost::asio::transfer_at_least(1),
        boost::bind(&tcp_connection::handle_read, shared_from_this(), 
            boost::asio::placeholders::error));
}


tcp_connection::tcp_connection(boost::asio::io_service& io_service, tcp_server* this_server)
    : socket_(io_service)
{
    unp.reserve_buffer();
	this->this_server = this_server;
}

void tcp_connection::handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
{
    // when using async_write this can close the connection.
}

void tcp_connection::handle_read(const boost::system::error_code& ec)
{
    if (!ec)
    {
        std::istream is(&input_buffer_);
        std::string line(std::istreambuf_iterator<char>(is), {});
    
        //Feed data into msgpack unpacker
        if(line.size() > unp.buffer_capacity())
            unp.reserve_buffer(line.size());

        memcpy(unp.buffer(), line.data(), line.size());
        unp.buffer_consumed(line.size());

        //Check if any objects are complete
        msgpack::object_handle result;
        while(unp.next(result)) {
            msgpack::object deserialized(result.get());
            dispatch( deserialized );
        }

        //Prepare for next read
        boost::asio::async_read(socket_, input_buffer_,
            boost::asio::transfer_at_least(1),
            boost::bind(&tcp_connection::handle_read, shared_from_this(), 
                boost::asio::placeholders::error));
    }
    else
    {
        if(ec == boost::asio::error::eof)
            std::cout << "Disconnected" << endl;	
        else
            std::cout << "Error: " << ec << "\n";
    }
}

void tcp_connection::dispatch(const msgpack::object &deserialized)
{
	std::cout << "request: " << deserialized << std::endl;
	if(deserialized.type == msgpack::type::ARRAY 
		and deserialized.via.array.size == 4) //This is manditory for msgpack-rpc format
	{
		//Check we have a valid msgpack-rpc message
		const msgpack::object &rpc_type = deserialized.via.array.ptr[0];
		if(rpc_type.type != msgpack::type::POSITIVE_INTEGER and rpc_type.type != msgpack::type::NEGATIVE_INTEGER)
			return;
		const msgpack::object &rpc_msgid = deserialized.via.array.ptr[1];
		if(rpc_msgid.type != msgpack::type::POSITIVE_INTEGER and rpc_msgid.type != msgpack::type::NEGATIVE_INTEGER)
			return;
		const msgpack::object &rpc_method = deserialized.via.array.ptr[2];
		if(rpc_method.type != msgpack::type::STR)
			return;
		const msgpack::object &rpc_params = deserialized.via.array.ptr[3];
		if(rpc_params.type != msgpack::type::ARRAY)
			return;

		string rpc_method_str(rpc_method.via.str.ptr, rpc_method.via.str.size);
		//cout << "rpc_type " << rpc_type.via.i64 << endl;
		//cout << "rpc_msgid " << rpc_msgid.via.u64 << endl;
		//cout << "rpc_method " << rpc_method_str << endl;
		//cout << "rpc_params array of length " << rpc_params.via.array.size << endl;

		//Demonstrate simple "sum" function
		if(rpc_method_str == "execute_serverless_function" and rpc_params.via.array.size == 3)
		{
			const msgpack::object &arg1 = rpc_params.via.array.ptr[0];
			const msgpack::object &arg2 = rpc_params.via.array.ptr[1];
			const msgpack::object &arg3 = rpc_params.via.array.ptr[2];

			int client_id = arg1.via.i64;
			int model_id = arg2.via.i64;
			string input_string(arg3.via.str.ptr, arg3.via.str.size);
			// call the enqueue request method of the server.
			// for now the request contains some arbitrary arguments a, b
			// which will be modified acc to what is needed
			// along with a shared pointer to the connection object which will
			// be passed along to keep the connection alive and finally write the output
			cout<<"calling enqueue on the server\n";
			this_server->enqueue_request(client_id, model_id, input_string, shared_from_this());
			
		}
		else
		{
			//Prepare unknown function error response
			msgpack::sbuffer sbuf;
			msgpack::packer<msgpack::sbuffer> packer(sbuf);
			packer.pack_array(4);
			packer.pack_int(1); //Responses have type 1
			packer.pack_unsigned_long_long(rpc_msgid.via.u64);
			string err = "Unknown method";
			packer.pack_str(err.size());
			packer.pack_str_body(err.c_str(), err.size());
			packer.pack_nil(); //No result data
	
			//Send response (synchronous, for simplicity)
			string outBuff(sbuf.data(), sbuf.size());
			boost::asio::write(socket(), boost::asio::buffer(outBuff));
		}
	}
}

