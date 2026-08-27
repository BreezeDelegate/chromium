// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_prefs.h"

#include "base/values.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "url/gurl.h"

namespace breeze_adblock {
namespace {

constexpr char kDisabledSitesPref[] = "breeze.adblock.disabled_sites";

}  // namespace

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterDictionaryPref(kDisabledSitesPref);
}

std::string SiteKey(const GURL& url) {
  if (!url.SchemeIsHTTPOrHTTPS() || url.host().empty()) {
    return {};
  }

  std::string site = net::registry_controlled_domains::GetDomainAndRegistry(
      url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
  if (site.empty()) {
    site = url.host();
  }
  return site;
}

bool IsEnabledForSite(const PrefService* prefs, const GURL& url) {
  if (!prefs) {
    return true;
  }
  const std::string site = SiteKey(url);
  return site.empty() || !prefs->GetDict(kDisabledSitesPref).contains(site);
}

void SetEnabledForSite(PrefService* prefs, const GURL& url, bool enabled) {
  if (!prefs) {
    return;
  }
  const std::string site = SiteKey(url);
  if (site.empty()) {
    return;
  }

  ScopedDictPrefUpdate update(prefs, kDisabledSitesPref);
  base::DictValue& disabled_sites = update.Get();
  if (enabled) {
    disabled_sites.Remove(site);
  } else {
    disabled_sites.Set(site, true);
  }
}

}  // namespace breeze_adblock
