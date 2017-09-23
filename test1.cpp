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
#include "snow.hpp"

class echo_session : public snow::session<uint32_t, uint32_t> {
public:
    explicit echo_session(boost::asio::io_service &ios)
            : snow::session<uint32_t, uint32_t>{ios} {
    }

    virtual boost::optional<uint32_t> process(const uint32_t &req) override {
        SNOW_LOG_TRACE("req {}", req);
        return {req + 10};
    }
};

class server : public snow::server<echo_session> {
public:

    virtual int check(const char *data, std::size_t size) const override {
        if (size >= 4) {
            return 4;
        } else {
            return 0;
        }
    }

    virtual std::string encode(const response_t &rsp) const override {
        SNOW_LOG_TRACE("rsp {}", rsp);
        return std::string((char *) &rsp, sizeof(rsp));
    }

    virtual request_t decode(const char *data, std::size_t size) const override {
        return *(uint32_t *) (data);
    }
};


int main(int argc, char *argv[]) {
    SNOW_LOG_INFO("test1 begin");
    try {
        server the_server;
        the_server.start();
    } catch (std::exception &e) {
        SNOW_LOG_INFO("Exception: {}", e.what());
    }
    SNOW_LOG_INFO("test1 end");

    return 0;
}