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

#ifndef STARBOARD_LINUX_X64X11_SYSTEM_PROPERTIES_H_
#define STARBOARD_LINUX_X64X11_SYSTEM_PROPERTIES_H_

#if SB_API_VERSION >= SB_HAS_STARTUP_URL_SIGNING_VERSION
extern const char kCertificationScope[];
extern const char kBase64EncodedCertificationSecret[];
#endif  // SB_API_VERSION >= SB_HAS_STARTUP_URL_SIGNING_VERSION

#endif  // STARBOARD_LINUX_X64X11_SYSTEM_PROPERTIES_H_
