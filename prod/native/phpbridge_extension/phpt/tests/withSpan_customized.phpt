--TEST--
getWithSpanMetadata - method with #[WithSpan('name', kind)] args
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip PHP 8.0+ required for Attributes"); ?>
--INI--
extension=/otel/phpbridge.so
--FILE--
<?php

declare(strict_types=1);

require('includes/withSpanStubs.inc');

use OpenTelemetry\API\Instrumentation\WithSpan;

// SpanKind::KIND_SERVER = 2, use literal to avoid needing SpanKind class
class OrderService
{
    #[WithSpan('order.process', 2)]
    public function processOrder(): void {}
}

new OrderService();

var_dump(getWithSpanMetadata('OrderService', 'processOrder'));

echo 'Test completed';
?>
--EXPECT--
array(4) {
  ["span_name"]=>
  string(13) "order.process"
  ["span_kind"]=>
  int(2)
  ["param_attributes"]=>
  array(0) {
  }
  ["prop_attributes"]=>
  array(0) {
  }
}
Test completed
