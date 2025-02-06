#include "common_audio/signal_processing/spl.h"

const int8_t kWebRtcSpl_CountLeadingZeros32_Table[64] = {
    32, 8,  17, -1, -1, 14, -1, -1, -1, 20, -1, -1, -1, 28, -1, 18,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  26, 25, 24,
    4,  11, 23, 31, 3,  7,  10, 16, 22, 30, -1, -1, 2,  6,  13, 9,
    -1, 15, -1, 21, -1, 29, 19, -1, -1, -1, -1, -1, 1,  27, 5,  12,
};

static const int16_t kCoefficients48To32[2][8] = {
    {778, -2050, 1087, 23285, 12903, -3783, 441, 222},
    {222, 441, -3783, 12903, 23285, 1087, -2050, 778}};

void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz* state) {
  memset(state->S_48_24, 0, 8 * sizeof(int32_t));
  memset(state->S_24_24, 0, 16 * sizeof(int32_t));
  memset(state->S_24_16, 0, 8 * sizeof(int32_t));
  memset(state->S_16_8, 0, 8 * sizeof(int32_t));
}

void WebRtcSpl_Resample48khzTo32khz(const int32_t* In, int32_t* Out, size_t K) {
  /////////////////////////////////////////////////////////////
  // Filter operation:
  //
  // Perform resampling (3 input samples -> 2 output samples);
  // process in sub blocks of size 3 samples.
  int32_t tmp;
  size_t m;

  for (m = 0; m < K; m++) {
    tmp = 1 << 14;
    tmp += kCoefficients48To32[0][0] * In[0];
    tmp += kCoefficients48To32[0][1] * In[1];
    tmp += kCoefficients48To32[0][2] * In[2];
    tmp += kCoefficients48To32[0][3] * In[3];
    tmp += kCoefficients48To32[0][4] * In[4];
    tmp += kCoefficients48To32[0][5] * In[5];
    tmp += kCoefficients48To32[0][6] * In[6];
    tmp += kCoefficients48To32[0][7] * In[7];
    Out[0] = tmp;

    tmp = 1 << 14;
    tmp += kCoefficients48To32[1][0] * In[1];
    tmp += kCoefficients48To32[1][1] * In[2];
    tmp += kCoefficients48To32[1][2] * In[3];
    tmp += kCoefficients48To32[1][3] * In[4];
    tmp += kCoefficients48To32[1][4] * In[5];
    tmp += kCoefficients48To32[1][5] * In[6];
    tmp += kCoefficients48To32[1][6] * In[7];
    tmp += kCoefficients48To32[1][7] * In[8];
    Out[1] = tmp;

    // update pointers
    In += 3;
    Out += 2;
  }
}

void WebRtcSpl_Resample48khzTo8khz(const int16_t* in,
                                   int16_t* out,
                                   WebRtcSpl_State48khzTo8khz* state,
                                   int32_t* tmpmem) {
  ///// 48 --> 24 /////
  // int16_t  in[480]
  // int32_t out[240]
  /////
  WebRtcSpl_DownBy2ShortToInt(in, 480, tmpmem + 256, state->S_48_24);

  ///// 24 --> 24(LP) /////
  // int32_t  in[240]
  // int32_t out[240]
  /////
  WebRtcSpl_LPBy2IntToInt(tmpmem + 256, 240, tmpmem + 16, state->S_24_24);

  ///// 24 --> 16 /////
  // int32_t  in[240]
  // int32_t out[160]
  /////
  // copy state to and from input array
  memcpy(tmpmem + 8, state->S_24_16, 8 * sizeof(int32_t));
  memcpy(state->S_24_16, tmpmem + 248, 8 * sizeof(int32_t));
  WebRtcSpl_Resample48khzTo32khz(tmpmem + 8, tmpmem, 80);

  ///// 16 --> 8 /////
  // int32_t  in[160]
  // int16_t out[80]
  /////
  WebRtcSpl_DownBy2IntToShort(tmpmem, 160, out, state->S_16_8);
}

uint32_t WebRtcSpl_DivU32U16(uint32_t num, uint16_t den) {
  // Guard against division with 0
  if (den != 0) {
    return (uint32_t)(num / den);
  } else {
    return (uint32_t)0xFFFFFFFF;
  }
}

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den) {
  // Guard against division with 0
  if (den != 0) {
    return (int32_t)(num / den);
  } else {
    return (int32_t)0x7FFFFFFF;
  }
}

int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den) {
  // Guard against division with 0
  if (den != 0) {
    return (int16_t)(num / den);
  } else {
    return (int16_t)0x7FFF;
  }
}

int32_t WebRtcSpl_DivResultInQ31(int32_t num, int32_t den) {
  int32_t L_num = num;
  int32_t L_den = den;
  int32_t div = 0;
  int k = 31;
  int change_sign = 0;

  if (num == 0)
    return 0;

  if (num < 0) {
    change_sign++;
    L_num = -num;
  }
  if (den < 0) {
    change_sign++;
    L_den = -den;
  }
  while (k--) {
    div <<= 1;
    L_num <<= 1;
    if (L_num >= L_den) {
      L_num -= L_den;
      div++;
    }
  }
  if (change_sign == 1) {
    div = -div;
  }
  return div;
}

int32_t WebRtcSpl_DivW32HiLow(int32_t num, int16_t den_hi, int16_t den_low) {
  int16_t approx, tmp_hi, tmp_low, num_hi, num_low;
  int32_t tmpW32;

  approx = (int16_t)WebRtcSpl_DivW32W16((int32_t)0x1FFFFFFF, den_hi);
  // result in Q14 (Note: 3FFFFFFF = 0.5 in Q30)

  // tmpW32 = 1/den = approx * (2.0 - den * approx) (in Q30)
  tmpW32 = (den_hi * approx << 1) + ((den_low * approx >> 15) << 1);
  // tmpW32 = den * approx

  // result in Q30 (tmpW32 = 2.0-(den*approx))
  tmpW32 = (int32_t)((int64_t)0x7fffffffL - tmpW32);

  // Store tmpW32 in hi and low format
  tmp_hi = (int16_t)(tmpW32 >> 16);
  tmp_low = (int16_t)((tmpW32 - ((int32_t)tmp_hi << 16)) >> 1);

  // tmpW32 = 1/den in Q29
  tmpW32 = (tmp_hi * approx + (tmp_low * approx >> 15)) << 1;

  // 1/den in hi and low format
  tmp_hi = (int16_t)(tmpW32 >> 16);
  tmp_low = (int16_t)((tmpW32 - ((int32_t)tmp_hi << 16)) >> 1);

  // Store num in hi and low format
  num_hi = (int16_t)(num >> 16);
  num_low = (int16_t)((num - ((int32_t)num_hi << 16)) >> 1);

  // num * (1/den) by 32 bit multiplication (result in Q28)

  tmpW32 =
      num_hi * tmp_hi + (num_hi * tmp_low >> 15) + (num_low * tmp_hi >> 15);

  // Put result in Q31 (convert from Q28)
  tmpW32 = WEBRTC_SPL_LSHIFT_W32(tmpW32, 3);

  return tmpW32;
}

int16_t WebRtcSpl_GetScalingSquare(int16_t* in_vector,
                                   size_t in_vector_length,
                                   size_t times) {
  int16_t nbits = WebRtcSpl_GetSizeInBits((uint32_t)times);
  size_t i;
  int16_t smax = -1;
  int16_t sabs;
  int16_t* sptr = in_vector;
  int16_t t;
  size_t looptimes = in_vector_length;

  for (i = looptimes; i > 0; i--) {
    sabs = (*sptr > 0 ? *sptr++ : -*sptr++);
    smax = (sabs > smax ? sabs : smax);
  }
  t = WebRtcSpl_NormW32(WEBRTC_SPL_MUL(smax, smax));

  if (smax == 0) {
    return 0;  // Since norm(0) returns 0
  } else {
    return (t > nbits) ? 0 : nbits - t;
  }
}

int32_t WebRtcSpl_Energy(int16_t* vector,
                         size_t vector_length,
                         int* scale_factor) {
  int32_t en = 0;
  size_t i;
  int scaling =
      WebRtcSpl_GetScalingSquare(vector, vector_length, vector_length);
  size_t looptimes = vector_length;
  int16_t* vectorptr = vector;

  for (i = 0; i < looptimes; i++) {
    en += (*vectorptr * *vectorptr) >> scaling;
    vectorptr++;
  }
  *scale_factor = scaling;

  return en;
}