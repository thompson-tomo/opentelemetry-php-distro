#pragma once

#include "WithSpanAttributes.h"

#include <Zend/zend_types.h>
#include <unordered_map>

namespace opentelemetry::php {

// Per-process singleton mapping function hash → WithSpanMetadata for
// attribute-based (#[WithSpan]) instrumentation.
//
// Lifecycle: populated once per function from registerObserverHandlers(),
// persists for the entire process lifetime — required because with opcache
// the Zend observer calls registerObserverHandlers() only ONCE per function
// per process (subsequent requests reuse the cached op_array with handlers
// already installed).  Clearing per-request (like hooksStorage_) would break
// attribute hooks on the second request.
//
// TODO: ZTS — concurrent emplace() and find() are not thread-safe.
//       Add std::shared_mutex when ZTS support is needed (same caveat as
//       InternalFunctionInstrumentationStorage).
class AttrHooksStorage {
public:
    static AttrHooksStorage &getInstance() {
        static AttrHooksStorage instance_;
        return instance_;
    }

    // Returns nullptr if not found.
    const WithSpanMetadata *find(zend_ulong hash) const {
        auto it = storage_.find(hash);
        if (it == storage_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    // No-op if hash is already present (idempotent — safe for non-opcache
    // environments where registerObserverHandlers may run per-request).
    void store(zend_ulong hash, WithSpanMetadata meta) {
        storage_.emplace(hash, std::move(meta));
    }

private:
    AttrHooksStorage() = default;
    std::unordered_map<zend_ulong, WithSpanMetadata> storage_;
};

} // namespace opentelemetry::php
