#include <iostream>
#include <msgpack.hpp>
#include <boost/array.hpp>
#include "rpc_client.h"

rpc_client::rpc_client(int client_id, std::string server_addr, std::string port)
    : io_service(), resolver(io_service), query(server_addr, port), socket(io_service){
    this->client_id = client_id;
    this->endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket, endpoint_iterator);

}

void rpc_client::call(std::string call_string){
    try{
        // Create a msg pack buffer that looks like [1,2,"run_model",[100,33]]
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_array(4);
        packer.pack_uint32(1); // some fields
        packer.pack_uint32(1); // some random fields

        // rpc call string to identify the rpc on the server
        packer.pack_str(call_string.size());
        packer.pack_str_body(call_string.c_str(), call_string.size());

        // arguments for the rpc
        packer.pack_array(3);
        int model_id = 2;
        std::string input_string = "SOME ARBITRARY STRING FOR NOW";
        packer.pack_int64(this->client_id);
        packer.pack_int64(model_id);
        packer.pack_str(input_string.size());
        packer.pack_str_body(input_string.c_str(), input_string.size());

        //Send request (synchronous, for simplicity)
        std::string outBuff(sbuf.data(), sbuf.size());
        boost::asio::write(socket, boost::asio::buffer(outBuff));

        // TODO: async_read and bind to lambda handler.
        boost::array<char, 256> buf;
        boost::system::error_code error;

        size_t len = socket.read_some(boost::asio::buffer(buf), error);

        msgpack::object_handle result;
        msgpack::unpack(result, buf.data(), len);
        const msgpack::object &arg1 = result.get().via.array.ptr[0];
        const msgpack::object &arg2 = result.get().via.array.ptr[1];
        std::cout << result.get() << std::endl;

        if (error == boost::asio::error::eof)
        ; //break; // Connection closed cleanly by peer.
        else if (error)
        throw boost::system::system_error(error); // Some other error.
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void rpc_client::loop_call(std::string call_string){
    while(1){
        this->call(call_string);
    }
}