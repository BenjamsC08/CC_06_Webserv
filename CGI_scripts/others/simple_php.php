<?php
header('Content-Type: text/plain; charset=UTF-8');

echo "PHP CGI OK\n";
echo "cwd=" . getcwd() . "\n";
echo "method=" . (isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : '') . "\n";
echo "query=" . (isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : '') . "\n";
echo "content_length=" . (isset($_SERVER['CONTENT_LENGTH']) ? $_SERVER['CONTENT_LENGTH'] : '') . "\n";
echo "raw_body=" . file_get_contents('php://input') . "\n";
