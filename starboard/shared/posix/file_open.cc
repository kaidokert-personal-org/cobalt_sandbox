// Copyright 2016 The Cobalt Authors. All Rights Reserved.
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

#if SB_API_VERSION < 17

#include "starboard/shared/posix/file_internal.h"

#include "starboard/shared/posix/impl/file_open.h"

SbFile SbFileOpen(const char* path,
                  int flags,
                  bool* out_created,
                  SbFileError* out_error) {
  return ::starboard::shared::posix::impl::FileOpen(path, flags, out_created,
                                                    out_error);
}

#endif  // SB_API_VERSION < 17
