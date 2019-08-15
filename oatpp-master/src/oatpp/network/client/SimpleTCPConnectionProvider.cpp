/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "./SimpleTCPConnectionProvider.hpp"

#include "oatpp/network/Connection.hpp"
#include "oatpp/core/data/stream/ChunkedBuffer.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

#include <fcntl.h>
#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace oatpp { namespace network { namespace client {

SimpleTCPConnectionProvider::SimpleTCPConnectionProvider(const oatpp::String& host, v_word16 port)
  : m_host(host)
  , m_port(port)
{
  setProperty(PROPERTY_HOST, m_host);
  setProperty(PROPERTY_PORT, oatpp::utils::conversion::int32ToStr(port));
}

std::shared_ptr<oatpp::data::stream::IOStream> SimpleTCPConnectionProvider::getConnection(){

  auto portStr = oatpp::utils::conversion::int32ToStr(m_port);

  struct addrinfo hints;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  struct addrinfo* result;
  auto res = getaddrinfo(m_host->c_str(), portStr->c_str(), &hints, &result);

  if (res != 0) {
    throw std::runtime_error("[oatpp::network::client::SimpleTCPConnectionProvider::getConnection()]. Error. Call to getaddrinfo() faild.");
  }

  if (result == nullptr) {
    throw std::runtime_error("[oatpp::network::client::SimpleTCPConnectionProvider::getConnection()]. Error. Call to getaddrinfo() returned no results.");
  }

  struct addrinfo* currResult = result;
  oatpp::data::v_io_handle clientHandle =- 1;

  while(currResult != nullptr) {

    clientHandle = socket(currResult->ai_family, currResult->ai_socktype, currResult->ai_protocol);

    if(clientHandle >= 0) {

      if(connect(clientHandle, currResult->ai_addr, currResult->ai_addrlen) == 0) {
        break;
      } else {
#if defined(WIN32) || defined(_WIN32)
		    ::closesocket(clientHandle);
#else
        ::close(clientHandle);
#endif
      }

    }

    currResult = currResult->ai_next;

  }

  freeaddrinfo(result);

  if(currResult == nullptr) {
    throw std::runtime_error("[oatpp::network::client::SimpleTCPConnectionProvider::getConnection()]: Error. Can't connect.");
  }

#ifdef SO_NOSIGPIPE
  int yes = 1;
  v_int32 ret = setsockopt(clientHandle, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(int));
  if(ret < 0) {
    OATPP_LOGD("[oatpp::network::client::SimpleTCPConnectionProvider::getConnection()]", "Warning. Failed to set %s for socket", "SO_NOSIGPIPE");
  }
#endif

  return oatpp::network::Connection::createShared(clientHandle);

}

oatpp::async::CoroutineStarterForResult<const std::shared_ptr<oatpp::data::stream::IOStream>&> SimpleTCPConnectionProvider::getConnectionAsync() {

  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<oatpp::data::stream::IOStream>&> {
  private:
    oatpp::String m_host;
    v_int32 m_port;
    oatpp::data::v_io_handle m_clientHandle;
  private:
    struct addrinfo* m_result;
    struct addrinfo* m_currentResult;
  public:

    ConnectCoroutine(const oatpp::String& host, v_int32 port)
      : m_host(host)
      , m_port(port)
      , m_result(nullptr)
    {}

    ~ConnectCoroutine() {
      if(m_result != nullptr) {
        freeaddrinfo(m_result);
      }
    }

    Action act() override {

      auto portStr = oatpp::utils::conversion::int32ToStr(m_port);

      struct addrinfo hints;

      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = 0;
      hints.ai_protocol = 0;

      // TODO make call to get addrinfo non-blocking !!!
      auto res = getaddrinfo(m_host->c_str(), portStr->c_str(), &hints, &m_result);
      if (res != 0) {
        return error<async::Error>(
          "[oatpp::network::client::SimpleTCPConnectionProvider::getConnectionAsync()]. Error. Call to getaddrinfo() faild.");
      }

      m_currentResult = m_result;

      if (m_result == nullptr) {
        return error<async::Error>(
          "[oatpp::network::client::SimpleTCPConnectionProvider::getConnectionAsync()]. Error. Call to getaddrinfo() returned no results.");
      }

      return yieldTo(&ConnectCoroutine::iterateAddrInfoResults);

    }


    Action iterateAddrInfoResults() {

      if(m_currentResult != nullptr) {

        m_clientHandle = socket(m_currentResult->ai_family, m_currentResult->ai_socktype, m_currentResult->ai_protocol);

        if (m_clientHandle < 0) { // TODO - For windows it should check for SOCKET_ERROR constant
          m_currentResult = m_currentResult->ai_next;
          return repeat();
        }

#if defined(WIN32) || defined(_WIN32)
        u_long flags = 1;
        ioctlsocket(m_clientHandle, FIONBIO, &flags);
#else
        fcntl(m_clientHandle, F_SETFL, O_NONBLOCK);
#endif
#ifdef SO_NOSIGPIPE
        int yes = 1;
        v_int32 ret = setsockopt(m_clientHandle, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(int));
        if(ret < 0) {
          OATPP_LOGD("[oatpp::network::client::SimpleTCPConnectionProvider::getConnectionAsync()]", "Warning. Failed to set %s for socket", "SO_NOSIGPIPE");
        }
#endif
        return yieldTo(&ConnectCoroutine::doConnect);
      }

      return error<Error>("[oatpp::network::client::SimpleTCPConnectionProvider::getConnectionAsync()]: Error. Can't connect.");

    }

    Action doConnect() {

      errno = 0;

      auto res = connect(m_clientHandle, m_currentResult->ai_addr, m_currentResult->ai_addrlen);

#if defined(WIN32) || defined(_WIN32)

      auto error = WSAGetLastError();

      if(res == 0 || error == WSAEISCONN) {
        return _return(oatpp::network::Connection::createShared(m_clientHandle));
      }
      if(error == WSAEWOULDBLOCK || error == WSAEINPROGRESS) {
        return ioWait(m_clientHandle, oatpp::async::Action::IOEventType::IO_EVENT_WRITE);
      } else if(error == WSAEINTR) {
        return ioRepeat(m_clientHandle, oatpp::async::Action::IOEventType::IO_EVENT_WRITE);
      }

	    ::closesocket(m_clientHandle);

#else

      if(res == 0 || errno == EISCONN) {
        return _return(oatpp::network::Connection::createShared(m_clientHandle));
      }
      if(errno == EALREADY || errno == EINPROGRESS) {
        return ioWait(m_clientHandle, oatpp::async::Action::IOEventType::IO_EVENT_WRITE);
      } else if(errno == EINTR) {
        return ioRepeat(m_clientHandle, oatpp::async::Action::IOEventType::IO_EVENT_WRITE);
      }

	    ::close(m_clientHandle);

#endif

      m_currentResult = m_currentResult->ai_next;
      return yieldTo(&ConnectCoroutine::iterateAddrInfoResults);

    }

  };

  return ConnectCoroutine::startForResult(m_host, m_port);

}

}}}
