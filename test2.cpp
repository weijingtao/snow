//
// Created by terrywei on 2017/9/3.
//

#include <cstdint>
#include <iostream>
#include <vector>
#include <boost/optional.hpp>
#include <string>
#include "snow.hpp"

static auto logger = spdlog::basic_logger_mt("test2", "./test2.txt", true);

class AddCodec : public snow::Codec<uint32_t, uint32_t> {
public:
    virtual int check(const char* data, std::size_t size) const override {
        logger->trace("check size {}", size);
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
    logger->set_level(spdlog::level::trace);
    logger->trace("test2 begin");
    for(int i = 0; i < 1; ++i) {
        boost::asio::io_service ios;
        boost::asio::spawn(ios, [&ios](boost::asio::yield_context yield) mutable {
            std::vector<std::unique_ptr<AddCodec>> adds;
            for (uint32_t i = 0; i < 1; ++i) {
                adds.emplace_back(new AddCodec);
                adds.back()->get_request() = i;
            }
            snow::Client client(ios, yield);
            client.request(adds, std::chrono::milliseconds(100));
            for (auto &add : adds) {
                logger->trace("request success, req {}, resp = {}", add->get_request(), add->get_response());
            }
        });
        ios.run();
    }
    logger->trace("test2 end");

    return 0;
}