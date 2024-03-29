// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/url_request/static_http_user_agent_settings.h"

namespace net {

StaticHttpUserAgentSettings::StaticHttpUserAgentSettings(
    const std::string& accept_language,
    const std::string& accept_charset,
    const std::string& user_agent)
    : accept_language_(accept_language),
      accept_charset_(accept_charset),
      user_agent_(user_agent) {
}

StaticHttpUserAgentSettings::~StaticHttpUserAgentSettings() {
}

const std::string& StaticHttpUserAgentSettings::GetAcceptLanguage() const {
  return accept_language_;
}

const std::string& StaticHttpUserAgentSettings::GetAcceptCharset() const {
  return accept_charset_;
}

const std::string& StaticHttpUserAgentSettings::GetUserAgent() const {
  return user_agent_;
}

}  // namespace net

