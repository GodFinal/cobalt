// Copyright 2019 The Cobalt Authors. All Rights Reserved.
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

#include "cobalt/debug/backend/tracing_agent.h"

#include <vector>

#include "base/bind.h"
#include "cobalt/script/script_debugger.h"

namespace cobalt {
namespace debug {
namespace backend {

namespace {
// Size in characters of JSON to batch dataCollected events.
constexpr size_t kDataCollectedSize = 24 * 1024;

// Parameter fields:
constexpr char kValue[] = "value";
constexpr char kCategories[] = "categories";

// Events:
constexpr char kDataCollected[] = "Tracing.dataCollected";
constexpr char kTracingComplete[] = "Tracing.tracingComplete";
}  // namespace

TracingAgent::TracingAgent(DebugDispatcher* dispatcher,
                           script::ScriptDebugger* script_debugger)
    : dispatcher_(dispatcher),
      script_debugger_(script_debugger),
      tracing_started_(false),
      collected_size_(0),
      ALLOW_THIS_IN_INITIALIZER_LIST(commands_(this)) {
  DCHECK(dispatcher_);

  commands_["Tracing.end"] = &TracingAgent::End;
  commands_["Tracing.start"] = &TracingAgent::Start;

  dispatcher_->AddDomain("Tracing", commands_.Bind());
}

void TracingAgent::End(const Command& command) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!tracing_started_) {
    command.SendErrorResponse(Command::kInvalidRequest, "Tracing not started");
    return;
  }
  tracing_started_ = false;
  command.SendResponse();

  script_debugger_->StopTracing();
}

void TracingAgent::Start(const Command& command) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (tracing_started_) {
    command.SendErrorResponse(Command::kInvalidRequest,
                              "Tracing already started");
    return;
  }
  tracing_started_ = true;

  JSONObject params = JSONParse(command.GetParams());

  // Parse comma-separated tracing categories parameter.
  std::vector<std::string> categories;
  std::string category_param;
  if (params->GetString(kCategories, &category_param)) {
    for (size_t pos = 0, comma; pos < category_param.size(); pos = comma + 1) {
      comma = category_param.find(',', pos);
      if (comma == std::string::npos) comma = category_param.size();
      std::string category = category_param.substr(pos, comma - pos);
      categories.push_back(category);
    }
  }

  script_debugger_->StartTracing(categories, this);

  command.SendResponse();
}

void TracingAgent::AppendTraceEvent(const std::string& trace_event_json) {
  // We initialize a new list into which we collect events both when we start,
  // and after each time it's released in |SendDataCollectedEvent|.
  if (!collected_events_) {
    collected_events_.reset(new base::ListValue());
  }

  JSONObject event = JSONParse(trace_event_json);
  if (event) {
    collected_events_->Append(event.release());
    collected_size_ += trace_event_json.size();
  }

  if (collected_size_ >= kDataCollectedSize) {
    SendDataCollectedEvent();
  }
}

void TracingAgent::FlushTraceEvents() {
  SendDataCollectedEvent();
  dispatcher_->SendEvent(kTracingComplete, JSONObject());
}

void TracingAgent::SendDataCollectedEvent() {
  if (collected_events_) {
    collected_size_ = 0;
    JSONObject params(new base::DictionaryValue());
    // Releasing the list into the value param avoids copying it.
    params->Set(kValue, collected_events_.release());
    dispatcher_->SendEvent(kDataCollected, params);
  }
}

}  // namespace backend
}  // namespace debug
}  // namespace cobalt
