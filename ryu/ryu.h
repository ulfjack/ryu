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
#ifndef RYU_H
#define RYU_H

#include "ryu/config.h"

RYU_EXTERN_C_BEGIN;
#ifdef RYU_HEADER_ONLY
RYU_NAMESPACE_BEGIN;
#endif

RYU_PUBLIC_FUNC int d2s_buffered_n(double f, char* result);
RYU_PUBLIC_FUNC void d2s_buffered(double f, char* result);
RYU_PUBLIC_FUNC char* d2s(double f);

RYU_PUBLIC_FUNC int f2s_buffered_n(float f, char* result);
RYU_PUBLIC_FUNC void f2s_buffered(float f, char* result);
RYU_PUBLIC_FUNC char* f2s(float f);

#ifdef RYU_HEADER_ONLY
RYU_NAMESPACE_END;
#endif
RYU_EXTERN_C_END;

#endif // RYU_H
