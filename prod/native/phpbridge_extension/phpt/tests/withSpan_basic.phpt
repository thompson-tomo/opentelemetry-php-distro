--TEST--
getWithSpanMetadata - method with #[WithSpan] (no args)
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip PHP 8.0+ required for Attributes"); ?>
--INI--
extension=/otel/phpbridge.so
--FILE--
<?php

declare(strict_types=1);

require('includes/withSpanStubs.inc');

use OpenTelemetry\API\Instrumentation\WithSpan;

class MyService
{
    #[WithSpan]
    public function doWork(): void {}
}

new MyService();

var_dump(getWithSpanMetadata('MyService', 'doWork'));

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
  array(0) {
  }
}
Test completed
