#include "WithSpanAttributes.h"

#include <Zend/zend_API.h>
#include <Zend/zend_attributes.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_types.h>

#include <algorithm>
#include <cstring>
#include <string_view>

namespace opentelemetry::php {

using namespace std::string_view_literals;

namespace {

static constexpr std::string_view withSpanLcName  = "opentelemetry\\api\\instrumentation\\withspan"sv;
static constexpr std::string_view spanAttrLcName  = "opentelemetry\\api\\instrumentation\\spanattribute"sv;

/// Finds an argument in an attribute by positional index or by name.
/// Returns the index into attr->args, or -1 if not found.
int findAttrArgIndex(zend_attribute *attr, uint32_t positionalIdx, std::string_view argName) {
    for (uint32_t i = 0; i < attr->argc; i++) {
        if (!attr->args[i].name && i == positionalIdx) {
            return static_cast<int>(i);
        }
        if (attr->args[i].name) {
            std::string_view name{ZSTR_VAL(attr->args[i].name), ZSTR_LEN(attr->args[i].name)};
            if (name == argName) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

} // namespace

bool hasWithSpanAttribute(zend_function *func) {
    if (func->common.type != ZEND_USER_FUNCTION) {
        return false;
    }
    if (!func->common.attributes) {
        return false;
    }
    return zend_get_attribute_str(func->common.attributes, withSpanLcName.data(), withSpanLcName.size()) != nullptr;
}

std::optional<WithSpanMetadata> readWithSpanMetadata(zend_function *func) {
    if (func->common.type != ZEND_USER_FUNCTION || !func->common.attributes) {
        return std::nullopt;
    }

    zend_attribute *attr = zend_get_attribute_str(
        func->common.attributes, withSpanLcName.data(), withSpanLcName.size());
    if (!attr) {
        return std::nullopt;
    }

    WithSpanMetadata metadata;
    zend_class_entry *scope = func->common.scope;

    // ---- Read span_name (positional arg 0, or named 'span_name') ----
    {
        int argIdx = findAttrArgIndex(attr, 0, "span_name");
        if (argIdx >= 0) {
            zval resolved;
            ZVAL_UNDEF(&resolved);
            if (zend_get_attribute_value(&resolved, attr, static_cast<uint32_t>(argIdx), scope) == SUCCESS) {
                if (Z_TYPE(resolved) == IS_STRING) {
                    metadata.spanName = std::string{ZSTR_VAL(Z_STR(resolved)), ZSTR_LEN(Z_STR(resolved))};
                }
                zval_ptr_dtor(&resolved);
            }
        }
    }

    // ---- Read span_kind (positional arg 1, or named 'span_kind') ----
    {
        int argIdx = findAttrArgIndex(attr, 1, "span_kind");
        if (argIdx >= 0) {
            zval resolved;
            ZVAL_UNDEF(&resolved);
            if (zend_get_attribute_value(&resolved, attr, static_cast<uint32_t>(argIdx), scope) == SUCCESS) {
                if (Z_TYPE(resolved) == IS_LONG) {
                    metadata.spanKind = static_cast<int64_t>(Z_LVAL(resolved));
                }
                zval_ptr_dtor(&resolved);
            }
        }
    }

    // ---- Read #[SpanAttribute] on parameters ----
    // Parameter attributes use offset = paramIndex + 1 (per Zend attribute spec).
    {
        uint32_t numArgs = func->common.num_args;
        for (uint32_t i = 0; i < numArgs; i++) {
            // zend_get_parameter_attribute_str takes a 0-based parameter index;
            // it adds 1 internally before comparing with attr->offset.
            zend_attribute *paramAttr = zend_get_parameter_attribute_str(
                func->common.attributes, spanAttrLcName.data(), spanAttrLcName.size(), i);
            if (!paramAttr) {
                continue;
            }

            std::string attrKey;

            // Read optional 'name' arg (positional 0, or named 'name')
            int nameArgIdx = findAttrArgIndex(paramAttr, 0, "name");
            if (nameArgIdx >= 0) {
                zval resolved;
                ZVAL_UNDEF(&resolved);
                if (zend_get_attribute_value(&resolved, paramAttr, static_cast<uint32_t>(nameArgIdx), scope) == SUCCESS) {
                    if (Z_TYPE(resolved) == IS_STRING) {
                        attrKey = std::string{ZSTR_VAL(Z_STR(resolved)), ZSTR_LEN(Z_STR(resolved))};
                    }
                    zval_ptr_dtor(&resolved);
                }
            }

            // Default: use the PHP parameter name
            if (attrKey.empty()) {
                auto &argInfo = func->op_array.arg_info[i];
                attrKey = std::string{ZSTR_VAL(argInfo.name), ZSTR_LEN(argInfo.name)};
            }

            metadata.paramAttributes.push_back({i, std::move(attrKey)});
        }
    }

    // ---- Read #[SpanAttribute] on class properties (requires a scope/class) ----
    if (scope) {
        zend_string *propName = nullptr;
        zval        *propZval = nullptr;

        ZEND_HASH_FOREACH_STR_KEY_VAL(&scope->properties_info, propName, propZval) {
            if (!propName || !propZval) {
                continue;
            }
            auto *propInfo = static_cast<zend_property_info *>(Z_PTR_P(propZval));
            if (!propInfo || !propInfo->attributes) {
                continue;
            }

            zend_attribute *propAttr = zend_get_attribute_str(
                propInfo->attributes, spanAttrLcName.data(), spanAttrLcName.size());
            if (!propAttr) {
                continue;
            }

            std::string attrKey;

            // Read optional 'name' arg
            int nameArgIdx = findAttrArgIndex(propAttr, 0, "name");
            if (nameArgIdx >= 0) {
                zval resolved;
                ZVAL_UNDEF(&resolved);
                if (zend_get_attribute_value(&resolved, propAttr, static_cast<uint32_t>(nameArgIdx), scope) == SUCCESS) {
                    if (Z_TYPE(resolved) == IS_STRING) {
                        attrKey = std::string{ZSTR_VAL(Z_STR(resolved)), ZSTR_LEN(Z_STR(resolved))};
                    }
                    zval_ptr_dtor(&resolved);
                }
            }

            // Default: use the PHP property name
            if (attrKey.empty()) {
                attrKey = std::string{ZSTR_VAL(propName), ZSTR_LEN(propName)};
            }

            metadata.propAttributes.push_back({
                std::string{ZSTR_VAL(propName), ZSTR_LEN(propName)},
                std::move(attrKey)
            });
        } ZEND_HASH_FOREACH_END();
    }

    return metadata;
}

} // namespace opentelemetry::php
