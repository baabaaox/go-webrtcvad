#ifndef COMMON_AUDIO_SIGNAL_PROCESSING_SPL_H_
#define COMMON_AUDIO_SIGNAL_PROCESSING_SPL_H_

#include <stdint.h>
#include <string.h>

#define WEBRTC_SPL_WORD16_MAX 32767

#define WEBRTC_SPL_MUL_16_32_RSFT11(a, b)          \
        (WEBRTC_SPL_MUL_16_16(a, (b) >> 16) * (1 << 5) + \
        (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x0200) >> 10))
#define WEBRTC_SPL_MUL_16_32_RSFT14(a, b)          \
        (WEBRTC_SPL_MUL_16_16(a, (b) >> 16) * (1 << 2) + \
        (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x1000) >> 13))
#define WEBRTC_SPL_MUL_16_32_RSFT15(a, b)            \
        ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) * (1 << 1)) + \
        (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x2000) >> 14))

#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) (WEBRTC_SPL_MUL_16_16(a, b) >> (c))

#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(a, b, c) \
  ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t)(((int32_t)1) << ((c)-1)))) >> (c))

// C + the 32 most significant bits of A * B
#define WEBRTC_SPL_SCALEDIFF32(A, B, C) \
  (C + (B >> 16) * A + (((uint32_t)(B & 0x0000FFFF) * A) >> 16))

#define WEBRTC_SPL_SAT(a, b, c) (b > a ? a : b < c ? c : b)

// Shifting with negative numbers allowed
// Positive means left shift
#define WEBRTC_SPL_SHIFT_W32(x, c) ((c) >= 0 ? (x) * (1 << (c)) : (x) >> -(c))

// Shifting with negative numbers not allowed
// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_LSHIFT_W32(x, c) ((x) << (c))

#define WEBRTC_SPL_RSHIFT_U32(x, c) ((uint32_t)(x) >> (c))

#define WEBRTC_SPL_RAND(a) ((int16_t)((((int16_t)a * 18816) >> 7) & 0x00007fff))

#define WEBRTC_SPL_MUL(a, b) ((int32_t)((int32_t)(a) * (int32_t)(b)))

extern const int8_t kWebRtcSpl_CountLeadingZeros32_Table[64];

static __inline int WebRtcSpl_CountLeadingZeros32_NotBuiltin(uint32_t n) {
  // Normalize n by rounding up to the nearest number that is a sequence of 0
  // bits followed by a sequence of 1 bits. This number has the same number of
  // leading zeros as the original n. There are exactly 33 such values.
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;

  // Multiply the modified n with a constant selected (by exhaustive search)
  // such that each of the 33 possible values of n give a product whose 6 most
  // significant bits are unique. Then look up the answer in the table.
  return kWebRtcSpl_CountLeadingZeros32_Table[(n * 0x8c0b2891) >> 26];
}

static __inline int WebRtcSpl_CountLeadingZeros32(uint32_t n) {
#ifdef __GNUC__
  return n == 0 ? 32 : __builtin_clz(n);
#else
  return WebRtcSpl_CountLeadingZeros32_NotBuiltin(n);
#endif
}

static __inline int16_t WebRtcSpl_GetSizeInBits(uint32_t n) {
  return 32 - WebRtcSpl_CountLeadingZeros32(n);
}

static __inline int16_t WebRtcSpl_NormW32(int32_t a) {
  return a == 0 ? 0 : WebRtcSpl_CountLeadingZeros32(a < 0 ? ~a : a) - 1;
}

static __inline int16_t WebRtcSpl_NormU32(uint32_t a) {
  return a == 0 ? 0 : WebRtcSpl_CountLeadingZeros32(a);
}

typedef struct {
  int32_t S_48_24[8];
  int32_t S_24_24[16];
  int32_t S_24_16[8];
  int32_t S_16_8[8];
} WebRtcSpl_State48khzTo8khz;

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den);

void WebRtcSpl_Resample48khzTo8khz(const int16_t* in,
                                   int16_t* out,
                                   WebRtcSpl_State48khzTo8khz* state,
                                   int32_t* tmpmem);

void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz* state);

int16_t WebRtcSpl_GetScalingSquare(int16_t* in_vector,
                                   size_t in_vector_length,
                                   size_t times);

int32_t WebRtcSpl_Energy(int16_t* vector,
                         size_t vector_length,
                         int* scale_factor);

#endif  // COMMON_AUDIO_SIGNAL_PROCESSING_SPL_H_