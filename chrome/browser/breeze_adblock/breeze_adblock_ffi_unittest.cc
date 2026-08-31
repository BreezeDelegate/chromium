// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_ffi.rs.h"

#include <string>

#include "testing/gtest/include/gtest/gtest.h"

namespace breeze_adblock {
namespace {

TEST(BreezeAdblockFfiTest, RejectsNonFilterListInput) {
  EXPECT_TRUE(compile_engine(rust::Str("<html>error</html>"),
                             rust::Str("not a filter list"))
                  .empty());
}

TEST(BreezeAdblockFfiTest, CompilesMinimalFilterLists) {
  constexpr char kEasyList[] =
      "[Adblock Plus 2.0]\n! Title: EasyList\n||ads.example^\n";
  constexpr char kEasyPrivacy[] =
      "[Adblock Plus 2.0]\n! Title: EasyPrivacy\n||tracker.example^\n";

  EXPECT_FALSE(compile_engine(rust::Str(kEasyList), rust::Str(kEasyPrivacy))
                   .empty());
}

}  // namespace
}  // namespace breeze_adblock
