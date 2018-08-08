// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

#include <math.h>
#include <inttypes.h>
#include <iostream>
#include <string.h>
#include <chrono>
#include <random>
#include <stdint.h>
#include <stdio.h>

#if defined(__linux__)
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ryu/ryu.h"
#include "third_party/double-conversion/double-conversion/utils.h"
#include "third_party/double-conversion/double-conversion/double-conversion.h"

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

static int bench32(int samples, int iterations, bool verbose) {
  char* bufferown = (char*) calloc(BUFFER_SIZE, sizeof(char));
  std::mt19937 mt32(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  init(mv1);
  init(mv2);
  int throwaway = 0;
  for (int i = 0; i < samples; i++) {
    const uint32_t r = mt32();
    float f = int32Bits2Float(r);

    steady_clock::time_point t1 = steady_clock::now();
    for (int j = 0; j < iterations; j++) {
      f2s_buffered(f, bufferown);
      throwaway += bufferown[2];
    }
    steady_clock::time_point t2 = steady_clock::now();
    double delta1 = duration_cast<nanoseconds>( t2 - t1 ).count() / (double) iterations;
    update(mv1, delta1);

    t1 = steady_clock::now();
    for (int j = 0; j < iterations; j++) {
      fcv(f);
      throwaway += buffer[2];
    }
    t2 = steady_clock::now();
    double delta2 = duration_cast<nanoseconds>( t2 - t1 ).count() / (double) iterations;
    update(mv2, delta2);
    if (verbose) {
      printf("%u,%lf,%lf\n", r, delta1, delta2);
    }

    char* own = bufferown;
    char* theirs = fcv(f);
    if (throwaway == 12345) {
      printf("Argh!\n");
    }
    if (strcmp(own, theirs) != 0) {
      printf("For %x %20s %20s\n", r, own, theirs);
    }
  }
  if (!verbose) {
    printf("32: %8.3f %8.3f     %8.3f %8.3f\n",
        mv1.mean, sqrt(variance(mv1)), mv2.mean, sqrt(variance(mv2)));
  }
  return throwaway;
}

static int bench64(int samples, int iterations, bool verbose) {
  char* bufferown = (char*) calloc(BUFFER_SIZE, sizeof(char));
  std::mt19937_64 mt64(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  init(mv1);
  init(mv2);
  int throwaway = 0;
  for (int i = 0; i < samples; i++) {
    const uint64_t r = mt64();
    double f = int64Bits2Double(r);

    steady_clock::time_point t1 = steady_clock::now();
    for (int j = 0; j < iterations; j++) {
      d2s_buffered(f, bufferown);
      throwaway += bufferown[2];
    }
    steady_clock::time_point t2 = steady_clock::now();
    double delta1 = duration_cast<nanoseconds>( t2 - t1 ).count() / (double) iterations;
    update(mv1, delta1);

    t1 = steady_clock::now();
    for (int j = 0; j < iterations; j++) {
      dcv(f);
      throwaway += buffer[2];
    }
    t2 = steady_clock::now();
    double delta2 = duration_cast<nanoseconds>( t2 - t1 ).count() / (double) iterations;
    update(mv2, delta2);
    if (verbose) {
      printf("%" PRIu64 ",%lf,%lf\n", r, delta1, delta2);
    }

    char* own = bufferown;
    char* theirs = dcv(f);
    if (strcmp(own, theirs) != 0) {
      printf("For %16" PRIX64 " %28s %28s\n", r, own, theirs);
    }
  }
  if (!verbose) {
    printf("64: %8.3f %8.3f     %8.3f %8.3f\n",
        mv1.mean, sqrt(variance(mv1)), mv2.mean, sqrt(variance(mv2)));
  }
  return throwaway;
}

int main(int argc, char** argv) {
#if defined(__linux__)
  // Also disable hyperthreading with something like this:
  // cat /sys/devices/system/cpu/cpu*/topology/core_id
  // sudo /bin/bash -c "echo 0 > /sys/devices/system/cpu/cpu6/online"
  cpu_set_t my_set;
  CPU_ZERO(&my_set);
  CPU_SET(2, &my_set);
  sched_setaffinity(getpid(), sizeof(cpu_set_t), &my_set);
#endif

  // By default, run both 32 and 64-bit benchmarks with 10000 samples and 1000 iterations each.
  bool run32 = true;
  bool run64 = true;
  int samples = 10000;
  int iterations = 1000;
  bool verbose = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-32") == 0) {
      run32 = true;
      run64 = false;
    } else if (strcmp(argv[i], "-64") == 0) {
      run32 = false;
      run64 = true;
    } else if (strcmp(argv[i], "-v") == 0) {
      verbose = true;
    } else if (strncmp(argv[i], "-samples=", 9) == 0) {
      sscanf(argv[i], "-samples=%i", &samples);
    } else if (strncmp(argv[i], "-iterations=", 12) == 0) {
      sscanf(argv[i], "-iterations=%i", &iterations);
    }
  }

  if (!verbose) {
    // No need to buffer the output if we're just going to print three lines.
    setbuf(stdout, NULL);
  }

  if (verbose) {
    printf("float_bits_as_int,ryu_time_in_ns,grisu3_time_in_ns\n");
  } else {
    printf("    Average & Stddev Ryu  Average & Stddev Grisu3\n");
  }
  int throwaway = 0;
  if (run32) {
    throwaway += bench32(samples, iterations, verbose);
  }
  if (run64) {
    throwaway += bench64(samples, iterations, verbose);
  }
  if (argc == 1000) {
    // Prevent the compiler from optimizing the code away.
    printf("%d\n", throwaway);
  }
  return 0;
}
