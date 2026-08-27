// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use adblock::lists::ParseOptions;
use adblock::request::Request;
use adblock::{Engine, FilterSet};

pub struct BreezeAdblockEngine {
    engine: Engine,
    loaded: bool,
}

#[cxx::bridge(namespace = "breeze_adblock")]
mod ffi {
    extern "Rust" {
        type BreezeAdblockEngine;

        fn new_engine(serialized: &[u8]) -> Box<BreezeAdblockEngine>;
        fn compile_engine(easylist: &str, easyprivacy: &str) -> Vec<u8>;
        fn is_loaded(self: &BreezeAdblockEngine) -> bool;
        fn should_block(
            self: &BreezeAdblockEngine,
            url: &str,
            hostname: &str,
            source_hostname: &str,
            request_type: &str,
            third_party: bool,
            method: &str,
        ) -> bool;
        fn cosmetic_css(
            self: &BreezeAdblockEngine,
            url: &str,
            hostname: &str,
            domain: &str,
        ) -> String;
        fn generic_cosmetic_css(
            self: &BreezeAdblockEngine,
            url: &str,
            hostname: &str,
            domain: &str,
            classes: &str,
            ids: &str,
        ) -> String;
    }
}

fn new_engine(serialized: &[u8]) -> Box<BreezeAdblockEngine> {
    let mut engine = Engine::default();
    let loaded = engine.deserialize(serialized).is_ok();
    Box::new(BreezeAdblockEngine { engine, loaded })
}

fn compile_engine(easylist: &str, easyprivacy: &str) -> Vec<u8> {
    if easylist.is_empty() || easyprivacy.is_empty() {
        return Vec::new();
    }

    let mut filters = FilterSet::new(false);
    filters.add_filter_list(easylist.to_owned(), ParseOptions::default());
    filters.add_filter_list(easyprivacy.to_owned(), ParseOptions::default());
    Engine::new_with_filter_set(filters).serialize()
}

impl BreezeAdblockEngine {
    fn is_loaded(&self) -> bool {
        self.loaded
    }

    fn should_block(
        &self,
        url: &str,
        hostname: &str,
        source_hostname: &str,
        request_type: &str,
        third_party: bool,
        method: &str,
    ) -> bool {
        if !self.loaded {
            return false;
        }
        let request = Request::preparsed(
            url,
            hostname,
            source_hostname,
            request_type,
            third_party,
            method,
        );
        self.engine.check_network_request(&request).should_block()
    }

    fn cosmetic_css(&self, url: &str, hostname: &str, domain: &str) -> String {
        if !self.loaded {
            return String::new();
        }
        let resources = self
            .engine
            .url_cosmetic_resources_preparsed(url, hostname, domain);
        selectors_to_css(resources.hide_selectors.into_iter().collect())
    }

    fn generic_cosmetic_css(
        &self,
        url: &str,
        hostname: &str,
        domain: &str,
        classes: &str,
        ids: &str,
    ) -> String {
        if !self.loaded {
            return String::new();
        }
        let resources = self
            .engine
            .url_cosmetic_resources_preparsed(url, hostname, domain);
        if resources.generichide {
            return String::new();
        }
        let selectors = self.engine.hidden_class_id_selectors(
            classes.lines().filter(|value| !value.is_empty()),
            ids.lines().filter(|value| !value.is_empty()),
            &resources.exceptions,
        );
        selectors_to_css(selectors)
    }
}

fn selectors_to_css(mut selectors: Vec<String>) -> String {
    if selectors.is_empty() {
        return String::new();
    }
    selectors.sort_unstable();
    selectors.dedup();

    let mut css = String::new();
    for selector in selectors {
        css.push_str(&selector);
        css.push_str("{display:none!important;}\n");
    }
    css
}
