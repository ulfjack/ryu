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

#ifndef __RYU
#define __RYU

#ifdef __cplusplus
extern "C" {
#endif

void d2s_buffered(double f, char* result);
char* d2s(double f);

void f2s_buffered(float f, char* result);
char* f2s(float f);

#ifdef __cplusplus
}
#endif

#endif // __RYU

