// Copyright 2018 Ulf Adams
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <math.h>
#include <iostream>
#include <string.h>
#include <chrono>
#include <stdint.h>
#include <stdio.h>

#include "ryu/ryu.h"
#include "third_party/double-conversion/double-conversion/utils.h"
#include "third_party/double-conversion/double-conversion/double-conversion.h"
#include "third_party/mersenne/random.h"

using double_conversion::StringBuilder;
using double_conversion::DoubleToStringConverter;
using namespace std::chrono;

static int BUFFER_SIZE = 40;
static char* buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
static DoubleToStringConverter converter(
    DoubleToStringConverter::Flags::EMIT_TRAILING_DECIMAL_POINT
        | DoubleToStringConverter::Flags::EMIT_TRAILING_ZERO_AFTER_POINT,
    "Infinity",
    "NaN",
    'E',
    7,
    7,
    0,
    0);
static StringBuilder builder(buffer, BUFFER_SIZE);

char* fcv(float value) {
  builder.Reset();
  converter.ToShortestSingle(value, &builder);
  return builder.Finalize();
}

char* dcv(double value) {
  builder.Reset();
  converter.ToShortest(value, &builder);
  return builder.Finalize();
}

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

typedef struct {
  int64_t n;
  double mean;
  double m2;
} mean_and_variance;

void init(mean_and_variance &mv) {
  mv.n = 0;
  mv.mean = 0;
  mv.m2 = 0;
}

void update(mean_and_variance &mv, double x) {
  int64_t n = ++mv.n;
  double d = x - mv.mean;
  mv.mean += d / n;
  double d2 = x - mv.mean;
  mv.m2 += d * d2;
}

double variance(mean_and_variance &mv) {
  return mv.m2 / (mv.n - 1);
}

extern int64_t timeTimeTime;

#include <sched.h>
#include <unistd.h>

#include <x86intrin.h>
static inline uint64_t rdtsc() {
  return __rdtsc();
}

static void bench32(int count) {
  char* bufferown = (char*) calloc(BUFFER_SIZE, sizeof(char));
  RandomInit(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  init(mv1);
  init(mv2);
  int64_t anything = 0;
  for (int i = 0; i < count; i++) {
    uint32_t r = RandomU32();
    float f = int32Bits2Float(r);

    int throwaway = 0;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for (int j = 0; j < 1000; j++) {
      f2s_buffered(f, bufferown);
      throwaway += strlen(bufferown);
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    int64_t delta1 = duration_cast<microseconds>( t2 - t1 ).count();
    update(mv1, delta1);

    t1 = high_resolution_clock::now();
    for (int j = 0; j < 1000; j++) {
      fcv(f);
      throwaway += strlen(buffer);
    }
    t2 = high_resolution_clock::now();
    int64_t delta2 = duration_cast<microseconds>( t2 - t1 ).count();
    update(mv2, delta2);
    anything += delta1 + delta2;
//    printf("%u,%ld,%ld\n", r,delta1, delta2);

    char* own = bufferown;
    char* theirs = fcv(f);
    if (throwaway == 12345) {
      printf("Argh!\n");
    }
    if (strlen(own) != strlen(theirs)) {
//    if (strcmp(own, theirs) != 0) {
      printf("For %x %20s %20s\n", r, own, theirs);
    }
  }
  printf("32: %8.3f %8.3f     %8.3f %8.3f        %9ld\n", mv1.mean, sqrt(variance(mv1)), mv2.mean, sqrt(variance(mv2)), anything);
}

static void bench64(int count) {
  char* bufferown = (char*) calloc(BUFFER_SIZE, sizeof(char));
  RandomInit(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  init(mv1);
  init(mv2);
  int64_t anything = 0;
  for (int i = 0; i < count; i++) {
    uint64_t r = RandomU64();
    double f = int64Bits2Double(r);

    int throwaway = 0;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for (int j = 0; j < 1000; j++) {
      d2s_buffered(f, bufferown);
      throwaway += bufferown[2];
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    int64_t delta1 = duration_cast<microseconds>( t2 - t1 ).count();
    update(mv1, delta1);

    t1 = high_resolution_clock::now();
    for (int j = 0; j < 1000; j++) {
      dcv(f);
      throwaway += buffer[2];
    }
    t2 = high_resolution_clock::now();
    int64_t delta2 = duration_cast<microseconds>( t2 - t1 ).count();
    update(mv2, delta2);
    anything += delta1 + delta2;
#ifdef RENDER_RESULTS
    printf("%lu,%ld,%ld\n", r,delta1, delta2);
#endif

    char* own = bufferown;
    char* theirs = dcv(f);
//    printf("For %16lx %28s %28s\n", r, own, theirs);
    if (throwaway == 12345) {
      printf("Argh!\n");
    }
#ifdef MATCH_GRISU3_OUTPUT
    if (strcmp(own, theirs) != 0) {
#else
    if (strlen(own) != strlen(theirs)) {
#endif
      printf("For %16lx %28s %28s\n", r, own, theirs);
    }
  }
  printf("64: %8.3f %8.3f     %8.3f %8.3f        %9ld\n", mv1.mean, sqrt(variance(mv1)), mv2.mean, sqrt(variance(mv2)), anything);
}

int main(int argc, char** argv) {
  // Also disable hyperthreading with something like this:
  // cat /sys/devices/system/cpu/cpu*/topology/core_id
  // sudo /bin/bash -c "echo 0 > /sys/devices/system/cpu/cpu6/online"
  cpu_set_t my_set;
  CPU_ZERO(&my_set);
  CPU_SET(2, &my_set);
  sched_setaffinity(getpid(), sizeof(cpu_set_t), &my_set);

  // By default, run both 32 and 64-bit benchmarks with 100000 iterations each.
  bool run32 = true;
  bool run64 = true;
  int count = 100000;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-32") == 0) {
      run32 = true;
      run64 = false;
    } else if (strcmp(argv[i], "-64") == 0) {
      run32 = false;
      run64 = true;
    } else if (strncmp(argv[i], "-count=", 7) == 0) {
      sscanf(argv[i], "-count=%i", &count);
    }
  }

  printf("    Average & Stddev Ryu  Average & Stddev Grisu3  (--------)\n");
  if (run32) {
    bench32(count);
  }
  if (run64) {
    bench64(count);
  }
  return 0;
}
