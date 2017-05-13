// Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef STARBOARD_SHARED_WIN32_APPLICATION_STUB_H_
#define STARBOARD_SHARED_WIN32_APPLICATION_STUB_H_

#include "starboard/configuration.h"
#include "starboard/shared/internal_only.h"
#include "starboard/shared/starboard/application.h"
#include "starboard/shared/starboard/queue_application.h"
#include "starboard/types.h"

namespace starboard {
namespace shared {
namespace win32 {

// Stub application engine using the generic queue and a stub implementation.
class ApplicationStub : public shared::starboard::QueueApplication {
 public:
  ApplicationStub();
  ~ApplicationStub() SB_OVERRIDE;

  static ApplicationStub* Get() {
    return static_cast<ApplicationStub*>(shared::starboard::Application::Get());
  }

 protected:
  // --- Application overrides ---
  void Initialize() SB_OVERRIDE;
  void Teardown() SB_OVERRIDE;

  // --- QueueApplication overrides ---
  bool MayHaveSystemEvents() SB_OVERRIDE;
  Event* PollNextSystemEvent() SB_OVERRIDE;
  Event* WaitForSystemEventWithTimeout(SbTime time) SB_OVERRIDE;
  void WakeSystemEventWait() SB_OVERRIDE;
};

}  // namespace win32
}  // namespace shared
}  // namespace starboard

#endif  // STARBOARD_SHARED_WIN32_APPLICATION_STUB_H_