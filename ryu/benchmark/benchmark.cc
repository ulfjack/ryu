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
#include <stdlib.h>

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

constexpr int BUFFER_SIZE = 40;
static char buffer[BUFFER_SIZE];
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

void fcv(float value) {
  builder.Reset();
  converter.ToShortestSingle(value, &builder);
  builder.Finalize();
}

void dcv(double value) {
  builder.Reset();
  converter.ToShortest(value, &builder);
  builder.Finalize();
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

struct mean_and_variance {
  int64_t n = 0;
  double mean = 0;
  double m2 = 0;

  void update(double x) {
    ++n;
    double d = x - mean;
    mean += d / n;
    double d2 = x - mean;
    m2 += d * d2;
  }

  double variance() const {
    return m2 / (n - 1);
  }

  double stddev() const {
    return sqrt(variance());
  }
};

class benchmark_options {
public:
  benchmark_options() = default;
  benchmark_options(const benchmark_options&) = delete;
  benchmark_options& operator=(const benchmark_options&) = delete;

  bool run32() const { return m_run32; }
  bool run64() const { return m_run64; }
  int samples() const { return m_samples; }
  int iterations() const { return m_iterations; }
  bool verbose() const { return m_verbose; }
  bool ryu_only() const { return m_ryu_only; }
  bool classic() const { return m_classic; }

  void parse(const char * const arg) {
    if (strcmp(arg, "-32") == 0) {
      m_run32 = true;
      m_run64 = false;
    } else if (strcmp(arg, "-64") == 0) {
      m_run32 = false;
      m_run64 = true;
    } else if (strcmp(arg, "-v") == 0) {
      m_verbose = true;
    } else if (strcmp(arg, "-ryu") == 0) {
      m_ryu_only = true;
    } else if (strcmp(arg, "-classic") == 0) {
      m_classic = true;
    } else if (strncmp(arg, "-samples=", 9) == 0) {
      if (sscanf(arg, "-samples=%i", &m_samples) != 1 || m_samples < 1) {
        fail(arg);
      }
    } else if (strncmp(arg, "-iterations=", 12) == 0) {
      if (sscanf(arg, "-iterations=%i", &m_iterations) != 1 || m_iterations < 1) {
        fail(arg);
      }
    } else {
      fail(arg);
    }
  }

private:
  void fail(const char * const arg) {
    printf("Unrecognized option '%s'.\n", arg);
    exit(EXIT_FAILURE);
  }

  // By default, run both 32 and 64-bit benchmarks with 10000 samples and 1000 iterations each.
  bool m_run32 = true;
  bool m_run64 = true;
  int m_samples = 10000;
  int m_iterations = 1000;
  bool m_verbose = false;
  bool m_ryu_only = false;
  bool m_classic = false;
};

float generate_float(std::mt19937& mt32) {
  uint32_t r = mt32();
  float f = int32Bits2Float(r);
  return f;
}

static int bench32(const benchmark_options& options) {
  char bufferown[BUFFER_SIZE];
  std::mt19937 mt32(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  int throwaway = 0;
  if (options.classic()) {
    for (int i = 0; i < options.samples(); ++i) {
      const float f = generate_float(mt32);

      auto t1 = steady_clock::now();
      for (int j = 0; j < options.iterations(); ++j) {
        f2s_buffered(f, bufferown);
        throwaway += bufferown[2];
      }
      auto t2 = steady_clock::now();
      double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
      mv1.update(delta1);

      double delta2 = 0.0;
      if (!options.ryu_only()) {
        t1 = steady_clock::now();
        for (int j = 0; j < options.iterations(); ++j) {
          fcv(f);
          throwaway += buffer[2];
        }
        t2 = steady_clock::now();
        delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
        mv2.update(delta2);
      }

      if (options.verbose()) {
        if (options.ryu_only()) {
          printf("%.6a,%f\n", f, delta1);
        } else {
          printf("%.6a,%f,%f\n", f, delta1, delta2);
        }
      }

      if (!options.ryu_only() && strcmp(bufferown, buffer) != 0) {
        printf("For %.6a %20s %20s\n", f, bufferown, buffer);
      }
    }
  } else {
    std::vector<float> vec(options.samples());
    for (int i = 0; i < options.samples(); ++i) {
      vec[i] = generate_float(mt32);
    }

    for (int j = 0; j < options.iterations(); ++j) {
      auto t1 = steady_clock::now();
      for (int i = 0; i < options.samples(); ++i) {
        f2s_buffered(vec[i], bufferown);
        throwaway += bufferown[2];
      }
      auto t2 = steady_clock::now();
      double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
      mv1.update(delta1);

      double delta2 = 0.0;
      if (!options.ryu_only()) {
        t1 = steady_clock::now();
        for (int i = 0; i < options.samples(); ++i) {
          fcv(vec[i]);
          throwaway += buffer[2];
        }
        t2 = steady_clock::now();
        delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
        mv2.update(delta2);
      }

      if (options.verbose()) {
        if (options.ryu_only()) {
          printf("%f\n", delta1);
        } else {
          printf("%f,%f\n", delta1, delta2);
        }
      }
    }
  }
  if (!options.verbose()) {
    printf("32: %8.3f %8.3f", mv1.mean, mv1.stddev());
    if (!options.ryu_only()) {
      printf("     %8.3f %8.3f", mv2.mean, mv2.stddev());
    }
    printf("\n");
  }
  return throwaway;
}

double generate_double(std::mt19937& mt32) {
  uint64_t r = mt32();
  r <<= 32;
  r |= mt32(); // calling mt32() in separate statements guarantees order of evaluation
  double f = int64Bits2Double(r);
  return f;
}

static int bench64(const benchmark_options& options) {
  char bufferown[BUFFER_SIZE];
  std::mt19937 mt32(12345);
  mean_and_variance mv1;
  mean_and_variance mv2;
  int throwaway = 0;
  if (options.classic()) {
    for (int i = 0; i < options.samples(); ++i) {
      const double f = generate_double(mt32);

      auto t1 = steady_clock::now();
      for (int j = 0; j < options.iterations(); ++j) {
        d2s_buffered(f, bufferown);
        throwaway += bufferown[2];
      }
      auto t2 = steady_clock::now();
      double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
      mv1.update(delta1);

      double delta2 = 0.0;
      if (!options.ryu_only()) {
        t1 = steady_clock::now();
        for (int j = 0; j < options.iterations(); ++j) {
          dcv(f);
          throwaway += buffer[2];
        }
        t2 = steady_clock::now();
        delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
        mv2.update(delta2);
      }

      if (options.verbose()) {
        if (options.ryu_only()) {
          printf("%.13a,%f\n", f, delta1);
        } else {
          printf("%.13a,%f,%f\n", f, delta1, delta2);
        }
      }

      if (!options.ryu_only() && strcmp(bufferown, buffer) != 0) {
        printf("For %.13a %28s %28s\n", f, bufferown, buffer);
      }
    }
  } else {
    std::vector<double> vec(options.samples());
    for (int i = 0; i < options.samples(); ++i) {
      vec[i] = generate_double(mt32);
    }

    for (int j = 0; j < options.iterations(); ++j) {
      auto t1 = steady_clock::now();
      for (int i = 0; i < options.samples(); ++i) {
        d2s_buffered(vec[i], bufferown);
        throwaway += bufferown[2];
      }
      auto t2 = steady_clock::now();
      double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
      mv1.update(delta1);

      double delta2 = 0.0;
      if (!options.ryu_only()) {
        t1 = steady_clock::now();
        for (int i = 0; i < options.samples(); ++i) {
          dcv(vec[i]);
          throwaway += buffer[2];
        }
        t2 = steady_clock::now();
        delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
        mv2.update(delta2);
      }

      if (options.verbose()) {
        if (options.ryu_only()) {
          printf("%f\n", delta1);
        } else {
          printf("%f,%f\n", delta1, delta2);
        }
      }
    }
  }
  if (!options.verbose()) {
    printf("64: %8.3f %8.3f", mv1.mean, mv1.stddev());
    if (!options.ryu_only()) {
      printf("     %8.3f %8.3f", mv2.mean, mv2.stddev());
    }
    printf("\n");
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

  benchmark_options options;

  for (int i = 1; i < argc; ++i) {
    options.parse(argv[i]);
  }

  if (!options.verbose()) {
    // No need to buffer the output if we're just going to print three lines.
    setbuf(stdout, NULL);
  }

  if (options.verbose()) {
    printf("%sryu_time_in_ns%s\n", options.classic() ? "hexfloat," : "", options.ryu_only() ? "" : ",grisu3_time_in_ns");
  } else {
    printf("    Average & Stddev Ryu%s\n", options.ryu_only() ? "" : "  Average & Stddev Grisu3");
  }
  int throwaway = 0;
  if (options.run32()) {
    throwaway += bench32(options);
  }
  if (options.run64()) {
    throwaway += bench64(options);
  }
  if (argc == 1000) {
    // Prevent the compiler from optimizing the code away.
    printf("%d\n", throwaway);
  }
  return 0;
}
