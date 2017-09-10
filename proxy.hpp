#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include <tuple>
#include <exception>
#include <boost/asio.hpp>
#include "Connection.hpp"
#include "Acceptor.hpp"
#include "log/log.hpp"
#include "utils/buffer.hpp"
#include "utils/type.hpp"
#include "spdlog/fmt/bundled/ostream.h"

namespace snow
{
    class proxy {
    public:
        typedef std::tuple<std::string, std::string, uint16_t>                                      end_point_type;

        proxy(boost::asio::io_service& ios)
             : m_ios(ios) {

        }

        int init(std::vector<end_point_type>& end_points) {
            for(auto& end_point : end_points) {
                if(std::string("tcp") == std::get<0>(end_point)) {
                    boost::asio::ip::tcp::endpoint listen_endpoint(boost::asio::ip::tcp::v4(), std::get<2>(end_point));
                    std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(m_ios, listen_endpoint);
                    acceptor->set_new_connecte_call_back(std::bind(&proxy::on_connect, this, std::placeholders::_1));
                    acceptor->start();
                } else if(std::string("udp") == std::get<0>(end_point)) {

                }
            }
            return 0;
        }

        void set_pkg_spliter(utils::pkg_split_t pkg_spliter) {
            m_pkg_spliter = pkg_spliter;
        }

        void set_request_dispatcher(utils::request_dispatch_t request_dispatcher) {
            m_request_dispatcher = request_dispatcher;
        }


    private:
        void on_connect(boost::asio::ip::tcp::socket& socket) {
            SNOW_LOG_TRACE("new Connection socket fd {}, local addr {}, peer addr {}",
                           socket.native(),
                           socket.local_endpoint(),
                           socket.remote_endpoint());
            std::make_shared<Connection>(socket, m_request_dispatcher, m_pkg_spliter, 100 * 1000)->start();
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
        utils::pkg_split_t                m_pkg_spliter;
        utils::request_dispatch_t         m_request_dispatcher;
    };
}