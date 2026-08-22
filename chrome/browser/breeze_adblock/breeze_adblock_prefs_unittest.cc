// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_prefs.h"

#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace breeze_adblock {
namespace {

TEST(BreezeAdblockPrefsTest, UsesRegistrableDomainAsSiteKey) {
  EXPECT_EQ("example.com", SiteKey(GURL("https://www.example.com/path")));
  EXPECT_EQ("example.co.uk", SiteKey(GURL("https://a.b.example.co.uk/")));
  EXPECT_EQ("127.0.0.1", SiteKey(GURL("http://127.0.0.1:8080/")));
  EXPECT_EQ("localhost", SiteKey(GURL("http://localhost/")));
  EXPECT_TRUE(SiteKey(GURL("chrome://settings/")).empty());
  EXPECT_TRUE(SiteKey(GURL("file:///tmp/example.html")).empty());
}

TEST(BreezeAdblockPrefsTest, DisablesAndReenablesWholeSite) {
  sync_preferences::TestingPrefServiceSyncable prefs;
  RegisterProfilePrefs(prefs.registry());

  const GURL www("https://www.example.com/");
  const GURL cdn("https://cdn.example.com/script.js");
  const GURL other("https://example.net/");

  EXPECT_TRUE(IsEnabledForSite(&prefs, www));
  EXPECT_TRUE(IsEnabledForSite(&prefs, cdn));

  SetEnabledForSite(&prefs, www, false);
  EXPECT_FALSE(IsEnabledForSite(&prefs, www));
  EXPECT_FALSE(IsEnabledForSite(&prefs, cdn));
  EXPECT_TRUE(IsEnabledForSite(&prefs, other));

  SetEnabledForSite(&prefs, cdn, true);
  EXPECT_TRUE(IsEnabledForSite(&prefs, www));
  EXPECT_TRUE(IsEnabledForSite(&prefs, cdn));
}

}  // namespace
}  // namespace breeze_adblock
