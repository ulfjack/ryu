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
#ifndef RYU2_H
#define RYU2_H

#include "ryu/config.h"

RYU_EXTERN_C_BEGIN;

#include <inttypes.h>

#ifdef RYU_HEADER_ONLY
RYU_NAMESPACE_BEGIN;
#endif

RYU_PUBLIC_FUNC int d2fixed_buffered_n(double d, uint32_t precision, char* result);
RYU_PUBLIC_FUNC void d2fixed_buffered(double d, uint32_t precision, char* result);
RYU_PUBLIC_FUNC char* d2fixed(double d, uint32_t precision);

RYU_PUBLIC_FUNC int d2exp_buffered_n(double d, uint32_t precision, char* result);
RYU_PUBLIC_FUNC void d2exp_buffered(double d, uint32_t precision, char* result);
RYU_PUBLIC_FUNC char* d2exp(double d, uint32_t precision);

#ifdef RYU_HEADER_ONLY
RYU_NAMESPACE_END;
#endif

RYU_EXTERN_C_END;

#endif // RYU2_H
