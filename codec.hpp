#pragma once

#include <string>

template <typename REQ, typename RSP>
class codec {
public:
    using request_t = REQ;
    using response_t = RSP;

    virtual int check(const char* data, std::size_t size) const = 0;

    virtual std::string encode(const request_t& req) const = 0;

    virtual response_t decode(const char* data, std::size_t size) const = 0;
};