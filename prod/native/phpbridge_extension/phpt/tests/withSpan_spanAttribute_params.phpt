--TEST--
getWithSpanMetadata - #[SpanAttribute] on method parameters (positional and aliased)
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

class UserService
{
    #[WithSpan]
    public function addUser(
        #[SpanAttribute] string $username,
        string $password,
        #[SpanAttribute('user.tag')] string $tag,
    ): void {}
}

new UserService();

var_dump(getWithSpanMetadata('UserService', 'addUser'));

echo 'Test completed';
?>
--EXPECT--
array(4) {
  ["span_name"]=>
  NULL
  ["span_kind"]=>
  NULL
  ["param_attributes"]=>
  array(2) {
    [0]=>
    array(2) {
      ["arg_index"]=>
      int(0)
      ["attr_key"]=>
      string(8) "username"
    }
    [1]=>
    array(2) {
      ["arg_index"]=>
      int(2)
      ["attr_key"]=>
      string(8) "user.tag"
    }
  }
  ["prop_attributes"]=>
  array(0) {
  }
}
Test completed
