// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_SERVICE_H_
#define CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_SERVICE_H_

#include <string>
#include <string_view>

class GURL;

namespace breeze_adblock {

void WarmUp();

bool ShouldBlock(const GURL& url,
                 const GURL& source_url,
                 std::string_view request_type,
                 std::string_view method);

std::string CosmeticCssForUrl(const GURL& url);
std::string GenericCosmeticCssForUrl(const GURL& url,
                                     std::string_view classes,
                                     std::string_view ids);

}  // namespace breeze_adblock

#endif  // CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_SERVICE_H_
