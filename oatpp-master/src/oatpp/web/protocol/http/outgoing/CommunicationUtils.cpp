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

#include "CommunicationUtils.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {
  
bool CommunicationUtils::headerEqualsCI_FAST(const oatpp::data::share::MemoryLabel& headerValue, const char* value) {
  v_int32 size = (v_int32) std::strlen(value);
  return size == headerValue.getSize() && oatpp::base::StrBuffer::equalsCI_FAST(headerValue.getData(), value, size);
}
  
v_int32 CommunicationUtils::considerConnectionState(const std::shared_ptr<protocol::http::incoming::Request>& request,
                                                    const std::shared_ptr<protocol::http::outgoing::Response>& response){
  
  auto outState = response->getHeaders().find(Header::CONNECTION);
  if(outState != response->getHeaders().end() && headerEqualsCI_FAST(outState->second, Header::Value::CONNECTION_UPGRADE)) {
    return CONNECTION_STATE_UPGRADE;
  }
  
  if(request) {
    /* Set keep-alive to value specified in the client's request, if no Connection header present in response. */
    /* Set keep-alive to value specified in response otherwise */
    auto it = request->getHeaders().find(Header::CONNECTION);
    if(it != request->getHeaders().end() && headerEqualsCI_FAST(it->second, Header::Value::CONNECTION_KEEP_ALIVE)) {
      if(outState != response->getHeaders().end()) {
        if(headerEqualsCI_FAST(outState->second, Header::Value::CONNECTION_KEEP_ALIVE)) {
          return CONNECTION_STATE_KEEP_ALIVE;
        } else {
          return CONNECTION_STATE_CLOSE;
        }
      } else {
        response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);
        return CONNECTION_STATE_KEEP_ALIVE;
      }
    }
    
    /* If protocol == HTTP/1.1 */
    /* Set HTTP/1.1 default Connection header value (Keep-Alive), if no Connection header present in response. */
    /* Set keep-alive to value specified in response otherwise */
    auto& protocol = request->getStartingLine().protocol;
    if(protocol.getData() != nullptr && headerEqualsCI_FAST(protocol, "HTTP/1.1")) {
      if(outState != response->getHeaders().end()) {
        if(headerEqualsCI_FAST(outState->second, Header::Value::CONNECTION_KEEP_ALIVE)) {
          return CONNECTION_STATE_KEEP_ALIVE;
        } else {
          return CONNECTION_STATE_CLOSE;
        }
      } else {
        response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);
        return CONNECTION_STATE_KEEP_ALIVE;
      }
    }
    
  }
  
  /* If protocol != HTTP/1.1 */
  /* Set default Connection header value (Close), if no Connection header present in response. */
  /* Set keep-alive to value specified in response otherwise */
  if(outState != response->getHeaders().end()) {
    if(headerEqualsCI_FAST(outState->second, Header::Value::CONNECTION_KEEP_ALIVE)) {
      return CONNECTION_STATE_KEEP_ALIVE;
    } else {
      return CONNECTION_STATE_CLOSE;
    }
  } else {
    response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_CLOSE);
    return CONNECTION_STATE_CLOSE;
  }
  
  return CONNECTION_STATE_CLOSE;
  
}
  
}}}}}
