<?php

$mode = isset($_GET['mode']) ? $_GET['mode'] : 'normal';

function out_body($title, $extra = '')
{
	echo "<!DOCTYPE html>\n";
	echo "<html><head><meta charset=\"UTF-8\"><title>CGI header test</title></head><body>\n";
	echo "<h1>" . htmlspecialchars($title, ENT_QUOTES, 'UTF-8') . "</h1>\n";
	echo "<p>This body starts after the blank line separating CGI headers from body.</p>\n";
	if ($extra !== '')
		echo "<pre>" . htmlspecialchars($extra, ENT_QUOTES, 'UTF-8') . "</pre>\n";
	echo "</body></html>\n";
}

if ($mode === 'normal')
{
	header('Content-Type: text/html; charset=UTF-8');
	header('X-CGI-Test: normal');
	header('X-CGI-Mode: normal');
	out_body('Normal mode');
	return;
}

if ($mode === 'status201')
{
	header('Status: 201 Created');
	header('Content-Type: text/html; charset=UTF-8');
	header('X-CGI-Test: status201');
	out_body('Status 201 Created');
	return;
}

if ($mode === 'status418')
{
	header("Status: 418 I'm a teapot");
	header('Content-Type: text/plain; charset=UTF-8');
	header('X-CGI-Test: status418');
	echo "If your parser is correct, HTTP status should become 418.\n";
	echo "Body begins only after the empty line after CGI headers.\n";
	return;
}

if ($mode === 'redirect')
{
	header('Status: 302 Found');
	header('Location: redirected.html');
	header('X-CGI-Test: redirect');
	header('Content-Type: text/html');
	echo "Redirect test body.\n";
	return;
}

if ($mode === 'cookies')
{
	header('Content-Type: text/plain; charset=UTF-8');
	header('Set-Cookie: alpha=one; Path=/', false);
	header('Set-Cookie: beta=two; Path=/', false);
	header('X-CGI-Test: cookies');
	echo "Two Set-Cookie headers should be preserved.\n";
	return;
}

if ($mode === 'multi')
{
	header('Content-Type: text/plain; charset=UTF-8');
	header('X-Alpha: one');
	header('X-Beta: two');
	header('X-Gamma: three');
	header('X-CGI-Test: multi');
	echo "Multiple simple headers test.\n";
	return;
}

if ($mode === 'bodycheck')
{
	header('Content-Type: text/plain; charset=UTF-8');
	header('X-CGI-Test: bodycheck');
	echo "line1\n";
	echo "line2\n";
	echo "\n";
	echo "line4_after_empty_line_inside_body\n";
	return;
}

header('Status: 400 Bad Request');
header('Content-Type: text/plain; charset=UTF-8');
echo "Unknown mode.\n";
echo "Use one of:\n";
echo "- ?mode=normal\n";
echo "- ?mode=status201\n";
echo "- ?mode=status418\n";
echo "- ?mode=redirect\n";
echo "- ?mode=cookies\n";
echo "- ?mode=multi\n";
echo "- ?mode=bodycheck\n";
