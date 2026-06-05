<?php

declare(strict_types=1);

namespace OTelDistroTests\ComponentTests;

use OpenTelemetry\API\Instrumentation\SpanAttribute;
use OpenTelemetry\API\Instrumentation\WithSpan;
use OpenTelemetry\API\Trace\SpanKind as OTelSpanKind;

/**
 * Test subject used by WithSpanAttributeTest — exercises all #[WithSpan] / #[SpanAttribute] combinations.
 */
final class WithSpanTestService
{
    #[SpanAttribute]
    public string $tenantId = '';

    #[WithSpan]
    public function basicMethod(): void
    {
    }

    #[WithSpan('custom.operation', OTelSpanKind::KIND_CLIENT)]
    public function customNameAndKind(): void
    {
    }

    #[WithSpan]
    public function withParamAttrs(
        #[SpanAttribute] string $userId,
        string $secret,
        #[SpanAttribute('request.operation')] string $operation,
    ): void {
    }

    #[WithSpan]
    public function withPropertyAttrs(): void
    {
    }

    #[WithSpan]
    public function throwingMethod(#[SpanAttribute] string $reason): void
    {
        throw new \RuntimeException("Deliberate: {$reason}");
    }
}
