#pragma once

#if _MSC_VER && !__INTEL_COMPILER
#define _WIN32_WINNT 0x0501
#endif

#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

namespace asio = boost::asio;
using ip_t = asio::ip::address_v6;
const static unsigned RAUSCHEN_MESSAGE_FORMAT_VERSION = 1;
const static int RAUSCHEN_MAX_PACKET_SIZE = 8192;

const static unsigned RAUSCHEN_PORT = 2442;
const static unsigned RAUSCHEN_BROADCAST_INTERVAL = 60;

const static char* RAUSCHEN_MULTICAST_ADDR = "FF05::DB8:80:4213";
