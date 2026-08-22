// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_COSMETIC_OBSERVER_H_
#define CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_COSMETIC_OBSERVER_H_

#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace breeze_adblock {

class BreezeAdblockCosmeticObserver
    : public content::WebContentsUserData<BreezeAdblockCosmeticObserver>,
      public content::WebContentsObserver {
 public:
  BreezeAdblockCosmeticObserver(const BreezeAdblockCosmeticObserver&) = delete;
  BreezeAdblockCosmeticObserver& operator=(
      const BreezeAdblockCosmeticObserver&) = delete;
  ~BreezeAdblockCosmeticObserver() override;

 private:
  friend class content::WebContentsUserData<BreezeAdblockCosmeticObserver>;

  explicit BreezeAdblockCosmeticObserver(content::WebContents* web_contents);

  void DOMContentLoaded(content::RenderFrameHost* render_frame_host) override;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace breeze_adblock

#endif  // CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_COSMETIC_OBSERVER_H_
