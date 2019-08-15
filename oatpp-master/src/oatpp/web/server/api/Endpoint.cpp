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

#include "Endpoint.hpp"

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"

namespace oatpp { namespace web { namespace server { namespace api {

Endpoint::Info::Param::Param()
  : name(nullptr)
  , type(nullptr)
{}

Endpoint::Info::Param::Param(const oatpp::String& pName,
                             oatpp::data::mapping::type::Type* pType)
  : name(pName)
  , type(pType)
{}

const std::list<oatpp::String>& Endpoint::Info::Params::getOrder() const {
  return m_order;
}

Endpoint::Info::Param& Endpoint::Info::Params::add(const oatpp::String& name, oatpp::data::mapping::type::Type* type) {
  m_order.push_back(name);
  Endpoint::Info::Param& param = operator [](name);
  param.name = name;
  param.type = type;
  return param;
}

Endpoint::Info::Param& Endpoint::Info::Params::operator [](const oatpp::String& name) {
  return m_params[name];
}

Endpoint::Info::Info()
{}

std::shared_ptr<Endpoint::Info> Endpoint::Info::createShared(){
  return std::make_shared<Info>();
}

oatpp::String Endpoint::Info::toString() {
  oatpp::data::stream::ChunkedBuffer stream;
  
  stream << "\nEndpoint\n";
  
  if(name) {
    stream << "name: '" << name << "'\n";
  }
  
  if(path){
    stream << "path: '" << path << "'\n";
  }
  
  if(method){
    stream << "method: '" << method << "'\n";
  }
  
  if(body.name != nullptr){
    stream << "body: '" << body.name << "', type: '" << body.type->name << "'\n";
  }
  
  auto headerIt = headers.getOrder().begin();
  while (headerIt != headers.getOrder().end()) {
    auto header = headers[*headerIt++];
    stream << "header: '" << header.name << "', type: '" << header.type->name << "'\n";
  }
  
  auto pathIt = pathParams.getOrder().begin();
  while (pathIt != pathParams.getOrder().end()) {
    auto param = pathParams[*pathIt++];
    stream << "pathParam: '" << param.name << "', type: '" << param.type->name << "'\n";
  }
  
  return stream.toString();
}

Endpoint::Endpoint(const std::shared_ptr<RequestHandler>& pHandler,
                   const std::shared_ptr<Info>& pInfo)
  : handler(pHandler)
  , info(pInfo)
{}

std::shared_ptr<Endpoint> Endpoint::createShared(const std::shared_ptr<RequestHandler>& handler,
                                                 const std::shared_ptr<Info>& info){
  return std::make_shared<Endpoint>(handler, info);
}

}}}}
