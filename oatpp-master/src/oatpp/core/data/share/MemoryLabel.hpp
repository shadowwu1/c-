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

#ifndef oatpp_data_share_MemoryLabel_hpp
#define oatpp_data_share_MemoryLabel_hpp

#include "oatpp/core/base/StrBuffer.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace data { namespace share {
  
/**
 * MemoryLabel represent a part of the whole memory buffer refered by handle.
 * Advantage of MemoryLabel use is that you may just "label" some data instead of allocating buffer for it's copy.
 * You may allocate separate buffer for data copy later once you need it.
 */
class MemoryLabel {
protected:
  std::shared_ptr<base::StrBuffer> m_memoryHandle;
  p_char8 m_data;
  v_int32 m_size;
public:

  /**
   * Default constructor. Null MemoryLabel.
   */
  MemoryLabel()
    : m_memoryHandle(nullptr)
    , m_data(nullptr)
    , m_size(0)
  {}

  /**
   * Constructor.
   * @param memHandle - memory handle. `std::shared_ptr` to buffer pointed by a memory label.
   * @param data - pointer to data.
   * @param size - size of the data in bytes.
   */
  MemoryLabel(const std::shared_ptr<base::StrBuffer>& memHandle, p_char8 data, v_int32 size);

  /**
   * Get pointer to labeled data.
   * @return - pointer to data.
   */
  p_char8 getData() const {
    return m_data;
  }

  /**
   * Get data size.
   * @return - size of the data.
   */
  v_int32 getSize() const {
    return m_size;
  }

  /**
   * Get memory handle which this memory label holds.
   * @return - `std::shared_ptr` to &id:oatpp::base::StrBuffer;.
   */
  std::shared_ptr<base::StrBuffer> getMemoryHandle() const {
    return m_memoryHandle;
  }

  /**
   * Check if labeled data equals to data specified.
   * Data is compared using &id:oatpp::base::StrBuffer::equals;.
   * @param data - data to compare with labeled data.
   * @return - `true` if equals.
   */
  bool equals(const char* data) const {
    v_int32 size = (v_int32) std::strlen(data);
    return m_size == size && base::StrBuffer::equals(m_data, data, m_size);
  }

  /**
   * Check if labeled data equals to data specified.
   * Data is compared using &id:oatpp::base::StrBuffer::equals;.
   * @param data - data to compare with labeled data.
   * @param size - data size.
   * @return - `true` if equals.
   */
  bool equals(const void* data, v_int32 size) const {
    return m_size == size && base::StrBuffer::equals(m_data, data, m_size);
  }

  /**
   * Create oatpp::String from memory label
   * @return oatpp::String(data, size)
   */
  oatpp::String toString() const {
    return oatpp::String((const char*) m_data, m_size, true);
  }

  /**
   * Create std::string from memory label
   * @return std::string(data, size)
   */
  std::string std_str() const {
    return std::string((const char*) m_data, m_size);
  }
  
};

/**
 * MemoryLabel which can be used as a key in unordered_map
 */
class StringKeyLabel : public MemoryLabel {
public:
  
  StringKeyLabel() : MemoryLabel() {};
  
  StringKeyLabel(const std::shared_ptr<base::StrBuffer>& memHandle, p_char8 data, v_int32 size);
  StringKeyLabel(const char* constText);
  StringKeyLabel(const oatpp::String& str);
  
  bool operator==(const StringKeyLabel &other) const {
    return m_size == other.m_size && base::StrBuffer::equals(m_data, other.m_data, m_size);
  }
  
  bool operator!=(const StringKeyLabel &other) const {
    return !(m_size == other.m_size && base::StrBuffer::equals(m_data, other.m_data, m_size));
  }
  
};

/**
 * MemoryLabel which can be used as a case-insensitive key in unordered_map
 */
class StringKeyLabelCI : public MemoryLabel {
public:
  
  StringKeyLabelCI() : MemoryLabel() {};
  
  StringKeyLabelCI(const std::shared_ptr<base::StrBuffer>& memHandle, p_char8 data, v_int32 size);
  StringKeyLabelCI(const char* constText);
  StringKeyLabelCI(const oatpp::String& str);
  
  bool operator==(const StringKeyLabelCI &other) const {
    return m_size == other.m_size && base::StrBuffer::equalsCI(m_data, other.m_data, m_size);
  }
  
  bool operator!=(const StringKeyLabelCI &other) const {
    return !(m_size == other.m_size && base::StrBuffer::equalsCI(m_data, other.m_data, m_size));
  }
  
};

/**
 * MemoryLabel which can be used as a case-insensitive-fast key in unordered_map.
 * CI_FAST - is appropriate for strings consisting of [a..z] + [A..Z] only.
 * for other symbols undefined collisions may occur.
 */
class StringKeyLabelCI_FAST : public MemoryLabel {
public:
  
  StringKeyLabelCI_FAST(const std::shared_ptr<base::StrBuffer>& memHandle, p_char8 data, v_int32 size);
  StringKeyLabelCI_FAST(const char* constText);
  StringKeyLabelCI_FAST(const oatpp::String& str);
  
  bool operator==(const StringKeyLabelCI_FAST &other) const {
    return m_size == other.m_size && base::StrBuffer::equalsCI_FAST(m_data, other.m_data, m_size);
  }
  
  bool operator!=(const StringKeyLabelCI_FAST &other) const {
    return !(m_size == other.m_size && base::StrBuffer::equalsCI_FAST(m_data, other.m_data, m_size));
  }
  
};
  
}}}

namespace std {
  
  template<>
  struct hash<oatpp::data::share::StringKeyLabel> {
    
    typedef oatpp::data::share::StringKeyLabel argument_type;
    typedef v_word32 result_type;
    
    result_type operator()(oatpp::data::share::StringKeyLabel const& s) const noexcept {
      
      p_char8 data = s.getData();
      v_int32 size4 = s.getSize() >> 2;
      
      result_type result = 0;
      
      for(v_int32 i = 0; i < size4; i++) {
        result ^= *((p_word32) data);
        data += 4;
      }
      
      for(v_int32 i = 0; i < s.getSize() - (size4 << 2); i++ ) {
        ((p_char8) &result)[i] ^= data[i];
      }
      
      return result;
    }
  };
  
  template<>
  struct hash<oatpp::data::share::StringKeyLabelCI> {
    
    typedef oatpp::data::share::StringKeyLabelCI argument_type;
    typedef v_word32 result_type;
    
    result_type operator()(oatpp::data::share::StringKeyLabelCI const& s) const noexcept {
      
      p_char8 data = s.getData();
      v_int32 size4 = s.getSize() >> 2;
      
      result_type result = 0;
      
      for(v_int32 i = 0; i < size4; i++) {
        result ^= (*((p_word32) data) | 538976288); // 538976288 = 32 | (32 << 8) | (32 << 16) | (32 << 24);
        data += 4;
      }
      
      for(v_int32 i = 0; i < s.getSize() - (size4 << 2); i++ ) {
        ((p_char8) &result)[i] ^= (data[i] | 32);
      }
      
      return result;
    }
  };
  
  template<>
  struct hash<oatpp::data::share::StringKeyLabelCI_FAST> {
    
    typedef oatpp::data::share::StringKeyLabelCI_FAST argument_type;
    typedef v_word32 result_type;
    
    result_type operator()(oatpp::data::share::StringKeyLabelCI_FAST const& s) const noexcept {
      
      p_char8 data = s.getData();
      v_int32 size4 = s.getSize() >> 2;
      
      result_type result = 0;
      
      for(v_int32 i = 0; i < size4; i++) {
        result ^= (*((p_word32) data) | 538976288); // 538976288 = 32 | (32 << 8) | (32 << 16) | (32 << 24);
        data += 4;
      }
      
      for(v_int32 i = 0; i < s.getSize() - (size4 << 2); i++ ) {
        ((p_char8) &result)[i] ^= (data[i] | 32);
      }
      
      return result;
    }
  };
  
}

#endif /* oatpp_data_share_MemoryLabel_hpp */
