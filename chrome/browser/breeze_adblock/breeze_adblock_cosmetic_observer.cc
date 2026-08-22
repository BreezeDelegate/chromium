// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_cosmetic_observer.h"

#include <string>

#include "base/functional/bind.h"
#include "base/json/string_escape.h"
#include "base/strings/strcat.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/breeze_adblock/breeze_adblock_service.h"
#include "chrome/common/chrome_isolated_world_ids.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace breeze_adblock {
namespace {

constexpr char kCollectDomTokensScript[] = R"JS(
(()=>{
  const classes=new Set(),ids=new Set(),limit=4096,maxLength=256;
  const walker=document.createTreeWalker(document,NodeFilter.SHOW_ELEMENT);
  for(let element=walker.nextNode();element;element=walker.nextNode()){
    if(ids.size<limit&&element.id&&element.id.length<=maxLength&&
       !/[\r\n]/.test(element.id))ids.add(element.id);
    if(classes.size<limit){
      for(const value of element.classList){
        if(value.length<=maxLength)classes.add(value);
        if(classes.size>=limit)break;
      }
    }
    if(classes.size>=limit&&ids.size>=limit)break;
  }
  return [Array.from(classes).join('\n'),Array.from(ids).join('\n')];
})()
)JS";

void InjectCss(content::RenderFrameHost* render_frame_host,
               const std::string& css) {
  if (css.empty()) {
    return;
  }
  const std::string script = base::StrCat(
      {"(()=>{const s=document.createElement('style');s.textContent=",
       base::GetQuotedJSONString(css),
       ";(document.head||document.documentElement).appendChild(s);})();"});
  render_frame_host->ExecuteJavaScriptInIsolatedWorld(
      base::UTF8ToUTF16(script), {}, ISOLATED_WORLD_ID_CHROME_INTERNAL);
}

void OnDomTokensCollected(content::GlobalRenderFrameHostId frame_id,
                          GURL url,
                          base::Value result) {
  content::RenderFrameHost* render_frame_host =
      content::RenderFrameHost::FromID(frame_id);
  if (!render_frame_host || render_frame_host->GetLastCommittedURL() != url ||
      !result.is_list()) {
    return;
  }
  const base::Value::List& values = result.GetList();
  if (values.size() != 2 || !values[0].is_string() || !values[1].is_string()) {
    return;
  }
  InjectCss(render_frame_host,
            GenericCosmeticCssForUrl(url, values[0].GetString(),
                                     values[1].GetString()));
}

}  // namespace

WEB_CONTENTS_USER_DATA_KEY_IMPL(BreezeAdblockCosmeticObserver);

BreezeAdblockCosmeticObserver::BreezeAdblockCosmeticObserver(
    content::WebContents* web_contents)
    : content::WebContentsUserData<BreezeAdblockCosmeticObserver>(*web_contents),
      content::WebContentsObserver(web_contents) {
  WarmUp();
}

BreezeAdblockCosmeticObserver::~BreezeAdblockCosmeticObserver() = default;

void BreezeAdblockCosmeticObserver::DOMContentLoaded(
    content::RenderFrameHost* render_frame_host) {
  const GURL& url = render_frame_host->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS()) {
    return;
  }

  InjectCss(render_frame_host, CosmeticCssForUrl(url));
  render_frame_host->ExecuteJavaScriptInIsolatedWorld(
      base::UTF8ToUTF16(kCollectDomTokensScript),
      base::BindOnce(&OnDomTokensCollected, render_frame_host->GetGlobalId(),
                     url),
      ISOLATED_WORLD_ID_CHROME_INTERNAL);
}

}  // namespace breeze_adblock
