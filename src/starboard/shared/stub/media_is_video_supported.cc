// Copyright 2017 The Cobalt Authors. All Rights Reserved.
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

#include "starboard/shared/starboard/media/media_support_internal.h"

#include "starboard/media.h"

SB_EXPORT bool SbMediaIsVideoSupported(SbMediaVideoCodec /*video_codec*/,
                                       int /*frame_width*/,
                                       int /*frame_height*/,
                                       int64_t /*bitrate*/,
                                       int /*fps*/
#if SB_API_VERSION >= 10
                                       ,
                                       bool /*decode_to_texture_required*/
#endif                                      // SB_API_VERSION >= 10
#if SB_HAS(MEDIA_EOTF_CHECK_SUPPORT)
                                       ,
                                       SbMediaTransferId /* eotf */
#endif  // SB_HAS(MEDIA_EOTF_CHECK_SUPPORT)
                                       ) {
  return false;
}
