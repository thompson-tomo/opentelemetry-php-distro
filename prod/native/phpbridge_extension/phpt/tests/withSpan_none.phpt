--TEST--
getWithSpanMetadata - function without WithSpan attribute returns NULL
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip PHP 8.0+ required for Attributes"); ?>
--INI--
extension=/otel/phpbridge.so
--FILE--
<?php

declare(strict_types=1);

require('includes/withSpanStubs.inc');

class PlainService
{
    public function noAttribute(string $arg): void {}
}

new PlainService();

var_dump(getWithSpanMetadata('PlainService', 'noAttribute'));

echo 'Test completed';
?>
--EXPECT--
NULL
Test completed
