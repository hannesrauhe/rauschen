#pragma once

#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

#include <asio.hpp>

using ip_t = asio::ip::address_v6;
const static unsigned RAUSCH_MESSAGE_FORMAT_VERSION = 1;

//config

const static unsigned RAUSCH_PORT = 2442;
const static unsigned RAUSCH_BROADCAST_INTERVAL = 10;
