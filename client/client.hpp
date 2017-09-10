#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <initializer_list>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "client/detail/do_request.hpp"
#include "client/detail/codec_base.hpp"
#include "client/detail/wait_group.hpp"
#include "client/codec.hpp"
#include "utils/fixed_size_buffer.hpp"
#include "log/log.hpp"
#include "Connection.hpp"


namespace snow {
    namespace client {
        class Client {
            template <typename Codec>
            class DoRequest;

        public:
            Client(boost::asio::io_service& ios, boost::asio::yield_context& yield)
                    : m_ios{ios}
                    , m_yield{yield} {
                SNOW_LOG_TRACE(__func__);
            }

            template <typename Codec>
            void request(std::vector<std::unique_ptr<Codec>>& codecs,
                         std::chrono::milliseconds timeout) {
                detail::WaitGroup wait_group{m_ios, m_yield};
                for(auto& codec : codecs)
                    _request(m_ios, wait_group, *codec);
                wait_group.wait(timeout);
            }

            void request(std::initializer_list<detail::CodecBase*> codecs,
                         std::chrono::milliseconds timeout) {
                detail::WaitGroup wait_group{m_ios, m_yield};
                for(auto codec : codecs)
                    _request(m_ios, wait_group, *codec);
                wait_group.wait(timeout);
            }

        private:
            void _request(boost::asio::io_service& ios, detail::WaitGroup& wait_group, detail::CodecBase& codec) {
                std::make_shared<detail::DoRequest>(m_ios, wait_group, codec)->start();
            }

        private:
            boost::asio::io_service&     m_ios;
            boost::asio::yield_context&  m_yield;
        };
    }
}