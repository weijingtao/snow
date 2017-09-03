//
// Created by terrywei on 2017/9/3.
//

//
// Created by weitao on 3/5/16.
//

#include <cstdint>
#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <boost/optional.hpp>
#include <string>
#include "client/client.hpp"
#include "client/codec.hpp"

class AddCodec : public snow::Codec<uint32_t, uint32_t> {
public:
    virtual int check(const char* data, std::size_t size) const override {
        if(size < sizeof(uint32_t)) {
            return 0;
        } else {
            return sizeof(uint32_t);
        }
    }

    virtual bool encode(snow::FixedSizeBuffer<4096>& buffer) const override {
        buffer.append((char*)&m_request, sizeof(m_request));
        return true;
    }

    virtual bool decode(snow::FixedSizeBuffer<4096>& buffer) override {
        m_response = *(uint32_t*)(buffer.read_index());
        buffer.increase_read_index(4);
        return true;
    }

    virtual std::string get_dest_addr() const override {
        return "127.0.0.1:10000";
    }
};

int main(int argc, char* argv[]) {
    SNOW_LOG_INFO << "test2 begin" << std::endl;
    boost::asio::io_service ios;
    boost::asio::spawn(ios, [&ios](boost::asio::yield_context yield) mutable {
        AddCodec add;
        add.get_request() = 3;
        snow::Client client(ios, yield);
        if(client.request<AddCodec>(add, std::chrono::milliseconds(100))) {
            SNOW_LOG_INFO << "request success, resp = " << add.get_response() << std::endl;
        } else {
            SNOW_LOG_INFO << "request fail" << std::endl;
        }
    });
    ios.run();
    SNOW_LOG_INFO << "test2 end" << std::endl;

    return 0;
}