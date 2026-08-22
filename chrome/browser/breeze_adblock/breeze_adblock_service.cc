// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/breeze_adblock/breeze_adblock_service.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/memory/ref_counted_memory.h"
#include "base/no_destructor.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/breeze_adblock/breeze_adblock_ffi.rs.h"
#include "chrome/grit/component_extension_resources.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/gurl.h"

namespace breeze_adblock {
namespace {

class EngineService {
 public:
  EngineService() {
    scoped_refptr<base::RefCountedMemory> serialized =
        ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
            IDR_BREEZE_ADBLOCK_ENGINE);

    base::ThreadPool::PostTaskAndReplyWithResult(
        FROM_HERE, {base::TaskPriority::USER_VISIBLE},
        base::BindOnce(
            [](scoped_refptr<base::RefCountedMemory> serialized) {
              if (!serialized) {
                return new_engine(rust::Slice<const uint8_t>());
              }
              return new_engine(rust::Slice<const uint8_t>(serialized->data(),
                                                           serialized->size()));
            },
            std::move(serialized)),
        base::BindOnce(&EngineService::OnEngineReady, base::Unretained(this)));
  }

  bool ShouldBlock(const GURL& url,
                   const GURL& source_url,
                   std::string_view request_type,
                   std::string_view method) const {
    const BreezeAdblockEngine* engine = engine_.load(std::memory_order_acquire);
    if (!engine) {
      return false;
    }
    const bool third_party =
        !net::registry_controlled_domains::SameDomainOrHost(
            url, source_url,
            net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
    return engine->should_block(
        rust::Str(url.spec()), rust::Str(url.host().data(), url.host().size()),
        rust::Str(source_url.host().data(), source_url.host().size()),
        rust::Str(request_type.data(), request_type.size()), third_party,
        rust::Str(method.data(), method.size()));
  }

  std::string CosmeticCssForUrl(const GURL& url) const {
    const BreezeAdblockEngine* engine = engine_.load(std::memory_order_acquire);
    if (!engine) {
      return {};
    }
    const std::string domain = DomainForUrl(url);
    return static_cast<std::string>(engine->cosmetic_css(
        rust::Str(url.spec()), rust::Str(url.host().data(), url.host().size()),
        rust::Str(domain)));
  }

  std::string GenericCosmeticCssForUrl(const GURL& url,
                                       std::string_view classes,
                                       std::string_view ids) const {
    const BreezeAdblockEngine* engine = engine_.load(std::memory_order_acquire);
    if (!engine) {
      return {};
    }
    const std::string domain = DomainForUrl(url);
    return static_cast<std::string>(engine->generic_cosmetic_css(
        rust::Str(url.spec()), rust::Str(url.host().data(), url.host().size()),
        rust::Str(domain), rust::Str(classes.data(), classes.size()),
        rust::Str(ids.data(), ids.size())));
  }

 private:
  static std::string DomainForUrl(const GURL& url) {
    std::string domain = net::registry_controlled_domains::GetDomainAndRegistry(
        url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
    if (domain.empty()) {
      domain = url.host();
    }
    return domain;
  }

  void OnEngineReady(rust::Box<BreezeAdblockEngine> engine) {
    engine_owner_ =
        std::make_unique<rust::Box<BreezeAdblockEngine>>(std::move(engine));
    engine_.store(&**engine_owner_, std::memory_order_release);
  }

  std::unique_ptr<rust::Box<BreezeAdblockEngine>> engine_owner_;
  std::atomic<const BreezeAdblockEngine*> engine_{nullptr};
};

EngineService& GetEngineService() {
  static base::NoDestructor<EngineService> service;
  return *service;
}

}  // namespace

void WarmUp() {
  GetEngineService();
}

bool ShouldBlock(const GURL& url,
                 const GURL& source_url,
                 std::string_view request_type,
                 std::string_view method) {
  return GetEngineService().ShouldBlock(url, source_url, request_type, method);
}

std::string CosmeticCssForUrl(const GURL& url) {
  return GetEngineService().CosmeticCssForUrl(url);
}

std::string GenericCosmeticCssForUrl(const GURL& url,
                                     std::string_view classes,
                                     std::string_view ids) {
  return GetEngineService().GenericCosmeticCssForUrl(url, classes, ids);
}

}  // namespace breeze_adblock
