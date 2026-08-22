// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_THROTTLE_H_
#define CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_THROTTLE_H_

#include <string>

#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "url/gurl.h"

namespace network {
struct ResourceRequest;
}

namespace breeze_adblock {

class BreezeAdblockThrottle : public blink::URLLoaderThrottle {
 public:
  explicit BreezeAdblockThrottle(const network::ResourceRequest& request);
  ~BreezeAdblockThrottle() override;

  void WillStartRequest(network::ResourceRequest* request,
                        bool* defer) override;
  void WillRedirectRequest(
      net::RedirectInfo* redirect_info,
      const network::mojom::URLResponseHead& response_head,
      bool* defer,
      network::HttpRequestHeadersUpdateParams* headers_update_params) override;

 private:
  void MaybeBlock(const GURL& url, std::string_view method);

  GURL source_url_;
  std::string request_type_;
  bool blockable_ = false;
};

}  // namespace breeze_adblock

#endif  // CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_THROTTLE_H_
