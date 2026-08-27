// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_PREFS_H_
#define CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_PREFS_H_

#include <string>

class GURL;
class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace breeze_adblock {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

std::string SiteKey(const GURL& url);
bool IsEnabledForSite(const PrefService* prefs, const GURL& url);
void SetEnabledForSite(PrefService* prefs, const GURL& url, bool enabled);

}  // namespace breeze_adblock

#endif  // CHROME_BROWSER_BREEZE_ADBLOCK_BREEZE_ADBLOCK_PREFS_H_
