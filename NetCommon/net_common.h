#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <algorithm>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>