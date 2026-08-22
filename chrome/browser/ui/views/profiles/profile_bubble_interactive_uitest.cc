// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/functional/callback_helpers.h"
#include "base/test/metrics/histogram_tester.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_test_util.h"
#include "chrome/browser/signin/dice_web_signin_interceptor.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/browser_window/public/browser_window_features.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/frame/toolbar_button_provider.h"
#include "chrome/browser/ui/views/profiles/dice_web_signin_interception_bubble_view.h"
#include "chrome/browser/ui/views/profiles/profile_menu_coordinator.h"
#include "chrome/browser/ui/views/profiles/profile_menu_view.h"
#include "chrome/browser/ui/views/profiles/profile_menu_view_base.h"
#include "chrome/browser/ui/views/toolbar/avatar_toolbar_button_interface.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/interactive_test_utils.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "content/public/test/browser_test.h"
#include "google_apis/gaia/core_account_id.h"
#include "google_apis/gaia/gaia_id.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/accelerators/accelerator.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/test/widget_test.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

class ProfileBubbleInteractiveUiTest : public InProcessBrowserTest {
 public:
  // Returns dummy parameters for the interception bubble.
  WebSigninInterceptor::Delegate::BubbleParameters GetTestBubbleParameters() {
    AccountInfo account;
    account.account_id = CoreAccountId::FromGaiaId(GaiaId("ID1"));
    AccountInfo primary_account;
    primary_account.account_id = CoreAccountId::FromGaiaId(GaiaId("ID2"));
    return WebSigninInterceptor::Delegate::BubbleParameters(
        WebSigninInterceptor::SigninInterceptionType::kMultiUser, account,
        primary_account);
  }

  // Waits until the bubble is displayed and focused, and returns the view that
  // was focused.
  views::View* WaitForFocus(views::DialogDelegate* bubble) {
    views::Widget* widget = bubble->GetWidget();
    EXPECT_TRUE(widget);
    views::test::WidgetVisibleWaiter(widget).Wait();
    views::View* focusable_view = bubble->GetInitiallyFocusedView();
    EXPECT_TRUE(focusable_view);
    ui_test_utils::WaitForViewFocus(browser(), focusable_view,
                                    /*focused=*/true);
    return focusable_view;
  }

  // Clears the focus on the current bubble.
  void ClearFocus(views::View* view) {
    EXPECT_TRUE(view->HasFocus());
    view->GetFocusManager()->ClearFocus();
    EXPECT_FALSE(view->HasFocus());
  }

  // Bring the focus back using the accessibility shortcut.
  void Refocus(views::View* view) {
    EXPECT_FALSE(view->HasFocus());
    // Mac uses Cmd-Option-ArrowDown, other platforms use F6.
#if BUILDFLAG(IS_MAC)
    ui::KeyboardCode key = ui::VKEY_DOWN;
    bool alt = true;
    bool command = true;
#else
    ui::KeyboardCode key = ui::VKEY_F6;
    bool alt = false;
    bool command = false;
#endif
    ASSERT_TRUE(ui_test_utils::SendKeyPressToWindowSync(
        browser()->GetWindow()->GetNativeWindow(), key, /*control=*/false,
        /*shift=*/false, alt, command));
    ui_test_utils::WaitForViewFocus(browser(), view, /*focused=*/true);
    EXPECT_TRUE(view->HasFocus());
  }
};

// Flaky. See crbug.com/464411959.
//
// TODO(crbug.com/464411959): Reenable it.
IN_PROC_BROWSER_TEST_F(ProfileBubbleInteractiveUiTest,
                       DISABLED_InterceptionBubbleFocus) {
  // Create the inteerception bubble, owned by the view hierarchy.
  DiceWebSigninInterceptionBubbleView* bubble =
      new DiceWebSigninInterceptionBubbleView(
          browser(),
          BrowserView::GetBrowserViewForBrowser(browser())
              ->toolbar_button_provider()
              ->GetAvatarToolbarButtonInterface()
              ->GetBubbleAnchor(*browser()),
          GetTestBubbleParameters(), base::DoNothing());
  views::Widget* widget = views::BubbleDialogDelegateView::CreateBubble(bubble);
  widget->Show();
  // The bubble takes focus when it is displayed.
  views::View* focused_view = WaitForFocus(bubble);
  ClearFocus(focused_view);
  // The bubble can be refocused using keyboard shortcuts.
  Refocus(focused_view);
  // Cleanup.
  widget->CloseWithReason(views::Widget::ClosedReason::kUnspecified);
}
