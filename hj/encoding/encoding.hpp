#ifndef ENCODING_HPP
#define ENCODING_HPP

#include <hj/encoding/bits.hpp>

#include <hj/encoding/bytes.hpp>

#include <hj/encoding/endian.hpp>

#ifdef HJ_ENABLE_FLATBUFFER
#include <hj/encoding/flatbuffers.hpp>
#endif

#include <hj/encoding/hex.hpp>

#include <hj/encoding/ini.hpp>

#include <hj/encoding/json.hpp>

#ifdef HJ_ENABLE_PROTOBUF
#include <hj/encoding/protobuf.hpp>
#endif

#include <hj/encoding/unicode.hpp>

#include <hj/encoding/utf8.hpp>

#ifdef HJ_ENABLE_XML
#include <hj/encoding/xml.hpp>
#endif

#ifdef HJ_ENABLE_YAML
#include <hj/encoding/yaml.hpp>
#endif

#endif