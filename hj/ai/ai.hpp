#ifndef AI_HPP
#define AI_HPP

#ifdef HJ_ENABLE_ASR
#include <hj/ai/asr.hpp>
#endif

#ifdef HJ_ENABLE_LLAMA
#include <hj/ai/llama.hpp>
#endif

#ifdef HJ_ENABLE_QRCODE
#include <hj/ai/qrcode.hpp>
#endif

#ifdef HJ_ENABLE_VECTOR_INDEX
#include <hj/ai/vector_index.hpp>
#endif

#endif // AI_HPP