//
// Created by weitao on 3/5/16.
//

#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <optional>
#include <string>
#include "snow.hpp"

class echo_codec : public codec<std::string, std::string> {
public:
     virtual int check(const char* data, std::size_t size) const override {
         return size;
    }

    virtual std::string encode(const request_t& req) const override {
        return req;
    }

    virtual response_t decode(const char* data, std::size_t size) const override {
        return std::string(data, size);
    }
};

class echo_session : public snow::session<echo_codec> {
public:
    explicit echo_session(boost::asio::io_service& ios)
        : snow::session<echo_codec>{ios} {
    }

    virtual std::optional<std::string> process(const std::string& req) override {
        std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << req << std::endl;
        return {req};
    }
};

class server : public snow::server<echo_session>
{
public:
    virtual int init() {
        std::tuple<std::string, std::string, uint16_t> end_point("tcp", "", 50000);
        std::vector<std::tuple<std::string, std::string, uint16_t>> end_point_vec;
        end_point_vec.push_back(std::move(end_point));
        m_proxy.init(end_point_vec);
    }

    virtual std::size_t pkg_check(const char* data, std::size_t len) {
        if(len >= 4)
            return 4;
        else
            return 0;
    }
};


int main(int argc, char* argv[]) {
    SNOW_LOG_INFO << "test1 begin" << std::endl;
    try
    {
        server the_server;
        the_server.init();
        the_server.start();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    SNOW_LOG_INFO << "test1 end" << std::endl;

    return 0;
}