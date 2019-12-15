#pragma once

// https://stackoverflow.com/a/12631415/5642066
#include <xercesc/dom/DOMDocument.hpp>
using XERCES_CPP_NAMESPACE::DOMDocument;

// workaround for 'TryEnterCriticalSection' compile error
#ifdef _WIN32
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x400
#  endif
// Exclude rarely-used stuff from Windows headers
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

// Gideon
#include <gideon/Common.h>

// SNE
#include <sne/server/utility/Profiler.h>
#include <sne/srpc/RpcContainers.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/stream/Streamable.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <sne/core/container/Containers.h>
#include <sne/database/ado/AdoConnection.h>

// Boost
#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4819)
#endif
#include <boost/mpl/vector.hpp>
#include <boost/type_traits.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#ifdef _MSC_VER
#  pragma warning (pop)
#endif

// CRT
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>

// https://github.com/boostorg/random/issues/49#issuecomment-450601746
#define BOOST_ALLOW_DEPRECATED_HEADERS

