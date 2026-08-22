// Copyright 2026 BreezeDelegate
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use adblock::request::Request;
use adblock::Engine;

pub struct BreezeAdblockEngine {
    engine: Engine,
    loaded: bool,
}

#[cxx::bridge(namespace = "breeze_adblock")]
mod ffi {
    extern "Rust" {
        type BreezeAdblockEngine;

        fn new_engine(serialized: &[u8]) -> Box<BreezeAdblockEngine>;
        fn should_block(
            self: &BreezeAdblockEngine,
            url: &str,
            hostname: &str,
            source_hostname: &str,
            request_type: &str,
            third_party: bool,
            method: &str,
        ) -> bool;
    }
}

fn new_engine(serialized: &[u8]) -> Box<BreezeAdblockEngine> {
    let mut engine = Engine::default();
    let loaded = engine.deserialize(serialized).is_ok();
    Box::new(BreezeAdblockEngine { engine, loaded })
}

impl BreezeAdblockEngine {
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
}
