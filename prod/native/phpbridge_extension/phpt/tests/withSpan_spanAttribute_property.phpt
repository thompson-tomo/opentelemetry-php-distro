--TEST--
getWithSpanMetadata - #[SpanAttribute] on class properties (default and aliased key)
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip PHP 8.0+ required for Attributes"); ?>
--INI--
extension=/otel/phpbridge.so
--FILE--
<?php

declare(strict_types=1);

require('includes/withSpanStubs.inc');

use OpenTelemetry\API\Instrumentation\WithSpan;
use OpenTelemetry\API\Instrumentation\SpanAttribute;

class InvoiceService
{
    #[SpanAttribute]
    public string $invoiceId = '';

    #[SpanAttribute('invoice.customer')]
    public string $customerId = '';

    public string $secret = '';

    #[WithSpan]
    public function issue(): void {}
}

new InvoiceService();

var_dump(getWithSpanMetadata('InvoiceService', 'issue'));

echo 'Test completed';
?>
--EXPECT--
array(4) {
  ["span_name"]=>
  NULL
  ["span_kind"]=>
  NULL
  ["param_attributes"]=>
  array(0) {
  }
  ["prop_attributes"]=>
  array(2) {
    [0]=>
    array(2) {
      ["prop_name"]=>
      string(9) "invoiceId"
      ["attr_key"]=>
      string(9) "invoiceId"
    }
    [1]=>
    array(2) {
      ["prop_name"]=>
      string(10) "customerId"
      ["attr_key"]=>
      string(16) "invoice.customer"
    }
  }
}
Test completed
