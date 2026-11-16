// Regression tests for the documented buffer sizes: the buffered functions
// must never write more than the corresponding RYU_*_BUFFER_SIZE bytes, and
// the malloc-based functions must allocate enough memory for any precision.

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ryu/ryu.h"
#include "third_party/gtest/gtest.h"

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

static uint64_t nextRandom(uint64_t* state) {
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

#define CANARY 0xAB
#define CANARY_SIZE 16

TEST(ApiBoundsTest, D2sBufferSize) {
  const double specials[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e308, -1e308, 5e-324,
                             HUGE_VAL, -HUGE_VAL, NAN};
  for (size_t i = 0; i < sizeof(specials) / sizeof(specials[0]); ++i) {
    const double f = specials[i];
    char* buffer = new char[RYU_D2S_BUFFER_SIZE + CANARY_SIZE];
    memset(buffer + RYU_D2S_BUFFER_SIZE, CANARY, CANARY_SIZE);
    const int len = d2s_buffered_n(f, buffer);
    d2s_buffered(f, buffer);
    EXPECT_LT((size_t) len, (size_t) RYU_D2S_BUFFER_SIZE);
    EXPECT_EQ('\0', buffer[len]);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[RYU_D2S_BUFFER_SIZE + k]);
    }
    delete[] buffer;
  }
  uint64_t state = 0x123456789abcdefull;
  for (int i = 0; i < 1000000; ++i) {
    const double f = int64Bits2Double(nextRandom(&state));
    char* buffer = new char[RYU_D2S_BUFFER_SIZE + CANARY_SIZE];
    memset(buffer + RYU_D2S_BUFFER_SIZE, CANARY, CANARY_SIZE);
    const int len = d2s_buffered_n(f, buffer);
    EXPECT_LT((size_t) len, (size_t) RYU_D2S_BUFFER_SIZE);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[RYU_D2S_BUFFER_SIZE + k]);
    }
    delete[] buffer;
  }
}

TEST(ApiBoundsTest, F2sBufferSize) {
  const float specials[] = {0.0f, -0.0f, 1.0f, -1.0f, 0.5f, 3.4028235e38f, -3.4028235e38f,
                            1.4e-45f, HUGE_VALF, -HUGE_VALF, NAN};
  for (size_t i = 0; i < sizeof(specials) / sizeof(specials[0]); ++i) {
    const float f = specials[i];
    char* buffer = new char[RYU_F2S_BUFFER_SIZE + CANARY_SIZE];
    memset(buffer + RYU_F2S_BUFFER_SIZE, CANARY, CANARY_SIZE);
    const int len = f2s_buffered_n(f, buffer);
    f2s_buffered(f, buffer);
    EXPECT_LT((size_t) len, (size_t) RYU_F2S_BUFFER_SIZE);
    EXPECT_EQ('\0', buffer[len]);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[RYU_F2S_BUFFER_SIZE + k]);
    }
    delete[] buffer;
  }
  uint64_t state = 0xfedcba9876543210ull;
  for (int i = 0; i < 1000000; ++i) {
    const float f = int32Bits2Float((uint32_t) nextRandom(&state));
    char* buffer = new char[RYU_F2S_BUFFER_SIZE + CANARY_SIZE];
    memset(buffer + RYU_F2S_BUFFER_SIZE, CANARY, CANARY_SIZE);
    const int len = f2s_buffered_n(f, buffer);
    EXPECT_LT((size_t) len, (size_t) RYU_F2S_BUFFER_SIZE);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[RYU_F2S_BUFFER_SIZE + k]);
    }
    delete[] buffer;
  }
}

TEST(ApiBoundsTest, D2fixedBufferSize) {
  const double values[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e308, -1e308, 1.7976931348623157e308,
                           5e-324, 1e-300, 123456789.123456789, HUGE_VAL, -HUGE_VAL, NAN};
  const uint32_t precisions[] = {0, 1, 6, 17, 100, 767, 1688};
  uint64_t state = 0xdeadbeefcafef00dull;
  for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
    for (size_t p = 0; p < sizeof(precisions) / sizeof(precisions[0]); ++p) {
      const size_t size = RYU_D2FIXED_BUFFER_SIZE(precisions[p]);
      char* buffer = new char[size + CANARY_SIZE];
      memset(buffer + size, CANARY, CANARY_SIZE);
      const int len = d2fixed_buffered_n(values[v], precisions[p], buffer);
      EXPECT_LT((size_t) len, size);
      for (size_t k = 0; k < CANARY_SIZE; ++k) {
        EXPECT_EQ(CANARY, (unsigned char) buffer[size + k]);
      }
      delete[] buffer;
    }
  }
  for (int i = 0; i < 10000; ++i) {
    const double d = int64Bits2Double(nextRandom(&state));
    const uint32_t precision = (uint32_t) (nextRandom(&state) % 1700);
    const size_t size = RYU_D2FIXED_BUFFER_SIZE(precision);
    char* buffer = new char[size + CANARY_SIZE];
    memset(buffer + size, CANARY, CANARY_SIZE);
    const int len = d2fixed_buffered_n(d, precision, buffer);
    EXPECT_LT((size_t) len, size);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[size + k]);
    }
    delete[] buffer;
  }
}

TEST(ApiBoundsTest, D2expBufferSize) {
  const double values[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e308, -1e308, 1.7976931348623157e308,
                           5e-324, 1e-300, 123456789.123456789, HUGE_VAL, -HUGE_VAL, NAN};
  const uint32_t precisions[] = {0, 1, 6, 17, 100, 767, 1688};
  uint64_t state = 0x0badf00d12345678ull;
  for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
    for (size_t p = 0; p < sizeof(precisions) / sizeof(precisions[0]); ++p) {
      const size_t size = RYU_D2EXP_BUFFER_SIZE(precisions[p]);
      char* buffer = new char[size + CANARY_SIZE];
      memset(buffer + size, CANARY, CANARY_SIZE);
      const int len = d2exp_buffered_n(values[v], precisions[p], buffer);
      EXPECT_LT((size_t) len, size);
      for (size_t k = 0; k < CANARY_SIZE; ++k) {
        EXPECT_EQ(CANARY, (unsigned char) buffer[size + k]);
      }
      delete[] buffer;
    }
  }
  for (int i = 0; i < 10000; ++i) {
    const double d = int64Bits2Double(nextRandom(&state));
    const uint32_t precision = (uint32_t) (nextRandom(&state) % 1700);
    const size_t size = RYU_D2EXP_BUFFER_SIZE(precision);
    char* buffer = new char[size + CANARY_SIZE];
    memset(buffer + size, CANARY, CANARY_SIZE);
    const int len = d2exp_buffered_n(d, precision, buffer);
    EXPECT_LT((size_t) len, size);
    for (size_t k = 0; k < CANARY_SIZE; ++k) {
      EXPECT_EQ(CANARY, (unsigned char) buffer[size + k]);
    }
    delete[] buffer;
  }
}

// The malloc-based d2fixed()/d2exp() used to allocate a fixed 2000 bytes,
// which overflows the heap for precision > ~1688 (issue #197). The allocation
// must now scale with the requested precision.
TEST(ApiBoundsTest, MallocVariantsLargePrecision) {
  char* fixed = d2fixed(1.5, 100000);
  // Cast to char* so gtest never has to stream a bare nullptr_t: on MSVC,
  // `os << nullptr` is ambiguous (error C2593) between operator<<(bool) and
  // operator<<(const void*), which broke the windows-latest CI build.
  ASSERT_NE(static_cast<char*>(nullptr), fixed);
  EXPECT_EQ(2 + 100000, strlen(fixed)); // "1." + 100000 digits
  EXPECT_EQ(0, strncmp(fixed, "1.5000", 6));
  free(fixed);

  char* exp = d2exp(1.5, 100000);
  ASSERT_NE(static_cast<char*>(nullptr), exp);
  EXPECT_EQ(1 + 1 + 100000 + 4, strlen(exp)); // "1." + 100000 digits + "e+00"
  free(exp);

  char* fixedMax = d2fixed(1.7976931348623157e308, 100);
  ASSERT_NE(static_cast<char*>(nullptr), fixedMax);
  EXPECT_EQ(309 + 1 + 100, strlen(fixedMax)); // 309 integer digits + '.' + 100 digits
  free(fixedMax);
}

TEST(ApiBoundsTest, D2fixedMatchesSnprintf) {
  const double values[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e308, 5e-324, 1e-300,
                           123456789.123456789, 9.999999999999999e22, 2.2250738585072014e-308};
  const uint32_t precisions[] = {0, 1, 6, 17};
  uint64_t state = 0xabcdef0123456789ull;
  char expected[RYU_D2FIXED_BUFFER_SIZE(17)];
  char actual[RYU_D2FIXED_BUFFER_SIZE(17)];
  for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
    for (size_t p = 0; p < sizeof(precisions) / sizeof(precisions[0]); ++p) {
      snprintf(expected, sizeof(expected), "%.*f", (int) precisions[p], values[v]);
      const int len = d2fixed_buffered_n(values[v], precisions[p], actual);
      actual[len] = '\0';
      EXPECT_STREQ(expected, actual);
    }
  }
  for (int i = 0; i < 10000; ++i) {
    const double d = int64Bits2Double(nextRandom(&state));
    if (!isfinite(d)) {
      continue;
    }
    const uint32_t precision = (uint32_t) (nextRandom(&state) % 18);
    snprintf(expected, sizeof(expected), "%.*f", (int) precision, d);
    const int len = d2fixed_buffered_n(d, precision, actual);
    actual[len] = '\0';
    EXPECT_STREQ(expected, actual);
  }
}

TEST(ApiBoundsTest, D2expMatchesSnprintf) {
  const double values[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e308, 5e-324, 1e-300,
                           123456789.123456789, 9.999999999999999e22, 2.2250738585072014e-308};
  const uint32_t precisions[] = {0, 1, 6, 17};
  uint64_t state = 0x9876543210fedcbaull;
  char expected[RYU_D2EXP_BUFFER_SIZE(17)];
  char actual[RYU_D2EXP_BUFFER_SIZE(17)];
  for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
    for (size_t p = 0; p < sizeof(precisions) / sizeof(precisions[0]); ++p) {
      snprintf(expected, sizeof(expected), "%.*e", (int) precisions[p], values[v]);
      const int len = d2exp_buffered_n(values[v], precisions[p], actual);
      actual[len] = '\0';
      EXPECT_STREQ(expected, actual);
    }
  }
  for (int i = 0; i < 10000; ++i) {
    const double d = int64Bits2Double(nextRandom(&state));
    if (!isfinite(d)) {
      continue;
    }
    const uint32_t precision = (uint32_t) (nextRandom(&state) % 18);
    snprintf(expected, sizeof(expected), "%.*e", (int) precision, d);
    const int len = d2exp_buffered_n(d, precision, actual);
    actual[len] = '\0';
    EXPECT_STREQ(expected, actual);
  }
}
