//
// Created by weitao on 3/5/16.
//

#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <boost/optional.hpp>
#include <string>
#include "snow.hpp"

class echo_session : public snow::session<std::string, std::string> {
public:
    explicit echo_session(boost::asio::io_service& ios)
        : snow::session<std::string, std::string>{ios} {
    }

    virtual boost::optional<std::string> process(const std::string& req) override {
        SNOW_LOG_TRACE << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << req << std::endl;
        return {req};
    }
};

class server : public snow::server<echo_session> {
public:

    virtual int check(const char* data, std::size_t size) const override {
        SNOW_LOG_TRACE << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << size << std::endl;
        if (size >= 10)
            return 10;
        else
            return 0;
    }

    virtual std::string encode(const response_t& req) const override {
        SNOW_LOG_TRACE << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << req << std::endl;
        return req;
    }

    virtual request_t decode(const char* data, std::size_t size) const override {
        SNOW_LOG_TRACE << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << size << std::endl;
        return std::string(data, size);
    }
};


int main(int argc, char* argv[]) {
    SNOW_LOG_INFO << "test1 begin" << std::endl;
    try {
        server the_server;
        the_server.start();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    SNOW_LOG_INFO << "test1 end" << std::endl;

    return 0;
}