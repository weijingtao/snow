#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <tuple>
#include <exception>
#include <boost/asio.hpp>
#include "connection.hpp"
#include "acceptor.hpp"
#include "log.hpp"
#include "buffer.hpp"

namespace snow
{
    class proxy
    {
    public:
        typedef std::tuple<std::string, std::string, uint16_t>                                      end_point_type;
        typedef std::function<void(const buffer&)>                                                  response_dispatch_type;
        typedef std::function<void(const char*, std::size_t, response_dispatch_type)>               request_dispatch_type;
        typedef std::function<int(const char*, std::size_t)>                                pkg_split_type;
        typedef std::unique_ptr<boost::asio::io_service::strand>                                    strand_ptr_t;
        typedef std::vector<strand_ptr_t>                                                           strand_vec_t;

        proxy(boost::asio::io_service& ios)
             : m_ios(ios) {

        }

        int init(std::vector<end_point_type>& end_points) {
            for(auto& end_point : end_points) {
                if(std::string("tcp") == std::get<0>(end_point)) {
                    m_acceptors.emplace_back(m_ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::get<2>(end_point)));
                } else if(std::string("udp") == std::get<0>(end_point)) {

                }
            }
            for(auto& acceptor : m_acceptors) {
                acceptor.set_new_connecte_call_back(std::bind(&proxy::on_connect, this, std::placeholders::_1));
            }
            for(auto& acceptor : m_acceptors) {
                boost::asio::spawn(acceptor.strand(), acceptor);
            }
            return 0;
        }

        void set_pkg_spliter(pkg_split_type pkg_spliter) {
            m_pkg_spliter = pkg_spliter;
        }

        void set_request_dispatcher(request_dispatch_type request_dispatcher) {
            m_request_dispatcher = request_dispatcher;
        }


    private:
        void on_connect(boost::asio::ip::tcp::socket& socket) {
            SNOW_LOG_TRACE << "new connection" << std::endl;
            std::make_shared<connection>(socket, m_request_dispatcher, m_pkg_spliter, 100)->start();
        }

        /*void create_udp_recever(const std::string& ip, uint16_t port) {
            strand_ptr_t strand(new boost::asio::io_service::strand(m_ios));
            boost::asio::spawn(*strand, [this, ip, port](boost::asio::yield_context yield) {
                auto socket = std::make_shared<boost::asio::ip::udp::socket>(m_ios, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));

                for (;;) {
                    boost::system::error_code ec;
                    std::array<char, 4096> recv_buffer;
                    boost::asio::ip::udp::endpoint peer;
                    std::size_t n_read = socket->async_receive_from(boost::asio::buffer(recv_buffer), peer, yield[ec]);
                    if (!ec) {
                        std::unique_ptr<request> req(new request(std::string(recv_buffer.data(), n_read)));
                        m_request_dispatcher(std::move(req), [&socket, &peer](const std::unique_ptr<response>&& rsp){
                            if(socket->is_open()) {
                                boost::asio::spawn(socket->get_io_service(), [&](boost::asio::yield_context yield) ->int {
                                    socket->async_send_to(boost::asio::buffer(rsp->get_data()), peer, yield);
                                });
                            }
                        });
                    }
                }
                socket->close();
            });
            m_strand_vec.push_back(std::move(strand));
        }*/

    private:
        boost::asio::io_service&          m_ios;
        std::vector<acceptor>             m_acceptors;
        pkg_split_type                    m_pkg_spliter;
        request_dispatch_type             m_request_dispatcher;
    };
}