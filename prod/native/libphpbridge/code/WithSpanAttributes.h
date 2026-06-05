#pragma once

#include <Zend/zend_types.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace opentelemetry::php {

/// Describes a function parameter annotated with #[SpanAttribute].
struct SpanAttributeParam {
    uint32_t argIndex; ///< 0-based parameter index
    std::string attrKey;  ///< span attribute key (from SpanAttribute::$name, or parameter name)
};

/// Describes a class property annotated with #[SpanAttribute].
struct SpanAttributeProp {
    std::string propName; ///< PHP property name
    std::string attrKey;  ///< span attribute key (from SpanAttribute::$name, or property name)
};

/// Metadata extracted from #[WithSpan] / #[SpanAttribute] attributes on a function/method.
/// Note: static attributes from WithSpan::$attributes are NOT stored here to avoid PHP value
/// lifecycle issues; they are re-read from func->common.attributes at call time.
struct WithSpanMetadata {
    std::optional<std::string> spanName;        ///< from WithSpan::$span_name
    std::optional<int64_t>     spanKind;        ///< from WithSpan::$span_kind
    std::vector<SpanAttributeParam> paramAttributes; ///< from #[SpanAttribute] on params
    std::vector<SpanAttributeProp>  propAttributes;  ///< from #[SpanAttribute] on properties
};

/// Returns true if the function/method has the #[WithSpan] attribute.
bool hasWithSpanAttribute(zend_function *func);

/// Reads WithSpan metadata from the function/method attributes.
/// Returns std::nullopt if #[WithSpan] is not present.
std::optional<WithSpanMetadata> readWithSpanMetadata(zend_function *func);

} // namespace opentelemetry::php
