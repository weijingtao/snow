#pragma once

#include <iostream>
/*
#include <boost/log/trivial.hpp>

#define SNOW_LOG_TRACE   BOOST_LOG_TRIVIAL(trace)
#define SNOW_LOG_DEBUG   BOOST_LOG_TRIVIAL(debug)
#define SNOW_LOG_INFO    BOOST_LOG_TRIVIAL(info)
#define SNOW_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define SNOW_LOG_ERROR   BOOST_LOG_TRIVIAL(error)
#define SNOW_LOG_FATAL   BOOST_LOG_TRIVIAL(fatal)*/

#define SNOW_LOG_TRACE   std::cerr
#define SNOW_LOG_DEBUG   std::cerr
#define SNOW_LOG_INFO    std::cerr
#define SNOW_LOG_WARNING std::cerr
#define SNOW_LOG_ERROR   std::cerr
#define SNOW_LOG_FATAL   std::cerr
