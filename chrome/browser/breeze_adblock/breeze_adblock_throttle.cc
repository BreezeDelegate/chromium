// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_throttle.h"

#include <string_view>

#include "chrome/browser/breeze_adblock/breeze_adblock_prefs.h"
#include "chrome/browser/breeze_adblock/breeze_adblock_service.h"
#include "net/base/net_errors.h"
#include "net/url_request/redirect_info.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/fetch_api.mojom-shared.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom-shared.h"
#include "url/origin.h"

namespace breeze_adblock {
namespace {

GURL GetSourceUrl(const network::ResourceRequest& request) {
  if (request.request_initiator && !request.request_initiator->opaque()) {
    return request.request_initiator->GetURL();
  }
  if (request.referrer.is_valid()) {
    return request.referrer;
  }
  return request.url;
}

std::string_view GetRequestType(const network::ResourceRequest& request) {
  if (request.url.SchemeIsWSOrWSS()) {
    return "websocket";
  }

  const auto resource_type =
      static_cast<blink::mojom::ResourceType>(request.resource_type);
  switch (resource_type) {
    case blink::mojom::ResourceType::kPing:
      return "ping";
    case blink::mojom::ResourceType::kXhr:
      return "xmlhttprequest";
    case blink::mojom::ResourceType::kCspReport:
      return "csp_report";
    default:
      break;
  }

  using Destination = network::mojom::RequestDestination;
  switch (request.destination) {
    case Destination::kDocument:
      return "document";
    case Destination::kFrame:
    case Destination::kIframe:
    case Destination::kFencedframe:
      return "subdocument";
    case Destination::kImage:
      return "image";
    case Destination::kScript:
      return "script";
    case Destination::kStyle:
      return "stylesheet";
    case Destination::kFont:
      return "font";
    case Destination::kAudio:
    case Destination::kTrack:
    case Destination::kVideo:
      return "media";
    case Destination::kEmbed:
    case Destination::kObject:
      return "object";
    case Destination::kReport:
      return "csp_report";
    case Destination::kEmpty:
      return request.is_fetch_like_api ? "xmlhttprequest" : "other";
    default:
      return "other";
  }
}

bool IsBlockable(const network::ResourceRequest& request) {
  return !request.is_outermost_main_frame &&
         request.destination != network::mojom::RequestDestination::kDocument;
}

}  // namespace

BreezeAdblockThrottle::BreezeAdblockThrottle(
    const network::ResourceRequest& request,
    const PrefService* prefs)
    : source_url_(GetSourceUrl(request)),
      request_type_(GetRequestType(request)),
      blockable_(IsBlockable(request)),
      enabled_for_site_(IsEnabledForSite(prefs, source_url_)) {}

BreezeAdblockThrottle::~BreezeAdblockThrottle() = default;

void BreezeAdblockThrottle::WillStartRequest(network::ResourceRequest* request,
                                             bool* defer) {
  if (blockable_ && enabled_for_site_) {
    MaybeBlock(request->url, request->method);
  }
}

void BreezeAdblockThrottle::WillRedirectRequest(
    net::RedirectInfo* redirect_info,
    const network::mojom::URLResponseHead& response_head,
    bool* defer,
    network::HttpRequestHeadersUpdateParams* headers_update_params) {
  if (blockable_ && enabled_for_site_) {
    MaybeBlock(redirect_info->new_url, redirect_info->new_method);
  }
}

void BreezeAdblockThrottle::MaybeBlock(const GURL& url,
                                       std::string_view method) {
  if (ShouldBlock(url, source_url_, request_type_, method)) {
    delegate_->CancelWithError(net::ERR_BLOCKED_BY_CLIENT, "breeze-adblock");
  }
}

}  // namespace breeze_adblock
