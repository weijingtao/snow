#pragma once

#include "log/log.hpp"
#include "server.hpp"
#include "session.hpp"
#include "client/codec.hpp"
#include "client/client.hpp"
#include "config.hpp"

namespace snow {
using Client = client::Client;

template<typename req, typename rsp>
using Codec = client::Codec<req, rsp>;
}
