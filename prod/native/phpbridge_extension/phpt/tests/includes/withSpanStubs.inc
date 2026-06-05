<?php

declare(strict_types=1);

// Minimal stubs for OpenTelemetry PHP attribute classes,
// so phpt tests can define #[WithSpan] / #[SpanAttribute] annotated classes
// without the full vendor present.

namespace OpenTelemetry\API\Instrumentation {

    #[Attribute(Attribute::TARGET_FUNCTION | Attribute::TARGET_METHOD)]
    class WithSpan
    {
        public function __construct(
            public readonly ?string $span_name = null,
            public readonly ?int    $span_kind = null,
            public readonly array   $attributes = [],
        ) {
        }
    }

    #[Attribute(Attribute::TARGET_PARAMETER | Attribute::TARGET_PROPERTY)]
    class SpanAttribute
    {
        public function __construct(
            public readonly ?string $name = null,
        ) {
        }
    }
}
