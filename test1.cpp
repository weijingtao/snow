//
// Created by weitao on 3/5/16.
//

#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <string>
#include "snow.hpp"

class echo_req : public snow::request_base
{
public:
    virtual bool parse_from_array(const char* req_data, std::size_t req_len) {
        m_data.append(req_data, req_len);
        return true;
    }

    virtual bool serialize_to_buffer(snow::buffer* rsp_buf) {
        return true;
    }
    std::string m_data;
};

class echo_rsp : public snow::response_base
{
public:
    virtual bool parse_from_array(const char* req_data, std::size_t req_len) {
        return true;
    }

    virtual bool serialize_to_buffer(snow::buffer* rsp_buf) {
        rsp_buf->ensure_writeable_bytes(m_data.size());
        rsp_buf->append(m_data.c_str(), m_data.length());
        return true;
    }
    std::string m_data;
};

class echo_session : public snow::session<echo_req, echo_rsp>
{
public:
    explicit echo_session(boost::asio::io_service& ios)
        : snow::session<echo_req, echo_rsp>(ios) {
    }

    virtual int process(const request_type& req, response_type* rsp) {
        rsp->m_data = req.m_data;
        auto self(shared_from_this());
        auto timer = std::make_shared<boost::asio::steady_timer>(m_strand.get_io_service());
        timer->expires_from_now(std::chrono::seconds(3));

        boost::asio::spawn(m_strand, [self, timer, this](boost::asio::yield_context yield) { timer->async_wait(yield); std::cout << "resume" << std::endl; resume(); } );
        std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << std::endl;
        yield();
        std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << std::endl;
//        resume();
        std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    }
};

class server : public snow::server_base<echo_session>
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