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

// clang-format off

// This file has been auto-generated by bindings/code_generator_cobalt.py. DO NOT MODIFY!
// Auto-generated from template: bindings/mozjs/templates/callback-interface.cc.template


#include "cobalt/bindings/testing/mozjs_single_operation_interface.h"
#include "cobalt/bindings/testing/arbitrary_interface.h"
#include "cobalt/bindings/testing/mozjs_arbitrary_interface.h"

#include "mozjs_gen_type_conversion.h"

#include "cobalt/script/logging_exception_state.h"
#include "cobalt/script/mozjs/conversion_helpers.h"
#include "cobalt/script/mozjs/mozjs_callback_interface.h"
#include "cobalt/script/mozjs/util/exception_helpers.h"
#include "cobalt/script/mozjs/util/stack_trace_helpers.h"
#include "third_party/mozjs/js/jsd/jsd.h"
#include "third_party/mozjs/js/src/jsapi.h"
#include "third_party/mozjs/js/src/jscntxt.h"

namespace {
using cobalt::bindings::testing::SingleOperationInterface;
using cobalt::bindings::testing::MozjsSingleOperationInterface;
using cobalt::bindings::testing::ArbitraryInterface;
using cobalt::bindings::testing::MozjsArbitraryInterface;

using cobalt::script::LoggingExceptionState;
using cobalt::script::mozjs::FromJSValue;
using cobalt::script::mozjs::GetCallableForCallbackInterface;
using cobalt::script::mozjs::ToJSValue;
using cobalt::script::mozjs::util::GetExceptionString;
}  // namespace

namespace cobalt {
namespace bindings {
namespace testing {

MozjsSingleOperationInterface::MozjsSingleOperationInterface(
    JSContext* context,
    JS::HandleObject implementing_object)
    : context_(context),
      implementing_object_(context, implementing_object) { }

MozjsSingleOperationInterface::MozjsSingleOperationInterface(
    JSContext* context,
    JS::HandleValue implementing_object_value)
    : context_(context),
      implementing_object_(context, implementing_object_value) { }

base::optional<int32_t > MozjsSingleOperationInterface::HandleCallback(
    const scoped_refptr<script::Wrappable>& callback_this,
    const scoped_refptr<ArbitraryInterface>& value,
    bool* had_exception) const {
  bool success = false;
  base::optional<int32_t > cobalt_return_value;
  JSAutoRequest auto_request(context_);
  AutoSaveExceptionState auto_save_exception_state(context_);
  ENABLE_JS_STACK_TRACE_IN_SCOPE(context_);

  // This could be set to NULL if it was garbage collected.
  JS::RootedObject implementing_object(context_, implementing_object_.GetObject());
  DLOG_IF(WARNING, !implementing_object) << "Implementing object is NULL.";
  if (implementing_object) {
    JSAutoCompartment auto_compartment(context_, implementing_object);

    // Get callable object.
    JS::RootedValue callable(context_);
    if (GetCallableForCallbackInterface(context_, implementing_object,
                                        "handleCallback", &callable)) {
      // Convert the callback_this to a JSValue.
      JS::RootedValue this_value(context_);
      ToJSValue(context_, callback_this, &this_value);

      // Convert arguments.
      const int kNumArguments = 1;
      JS::Value args[kNumArguments];
      js::SetValueRangeToNull(args, kNumArguments);
      js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
      ToJSValue(context_, value,
                auto_array_rooter.handleAt(0));

      // Call the function.
      JS::RootedValue return_value(context_);
      JS::RootedFunction function(
          context_, JS_ValueToFunction(context_, callable));
      DCHECK(function);
      success = JS::Call(context_, this_value, function, kNumArguments, args,
                         return_value.address());
      DLOG_IF(WARNING, !success) << "Exception in callback: "
                                 << GetExceptionString(context_);
      if (success) {
        LoggingExceptionState exception_state;
        FromJSValue(context_, return_value, 0, &exception_state,
                    &cobalt_return_value);
        success = !exception_state.is_exception_set();
      }
    }
  }

  *had_exception = !success;
  return cobalt_return_value;
}

}  // namespace testing
}  // namespace bindings
}  // namespace cobalt

