<?php

header('Content-Type: text/html; charset=UTF-8');
header('X-CGI-Test: php-ok');

function h($value)
{
	return htmlspecialchars((string)$value, ENT_QUOTES, 'UTF-8');
}

function print_array_html($title, $array)
{
	echo '<h2>' . h($title) . '</h2>';
	if (empty($array))
	{
		echo "<p><em>empty</em></p>\n";
		return;
	}
	echo "<pre>";
	foreach ($array as $key => $value)
	{
		if (is_array($value))
			echo h($key) . ' => ' . h(print_r($value, true)) . "\n";
		else
			echo h($key) . ' => ' . h($value) . "\n";
	}
	echo "</pre>";
}

$raw_body = file_get_contents('php://input');

$server_keys = array(
	'GATEWAY_INTERFACE',
	'REQUEST_METHOD',
	'REQUEST_URI',
	'QUERY_STRING',
	'CONTENT_TYPE',
	'CONTENT_LENGTH',
	'SCRIPT_NAME',
	'SCRIPT_FILENAME',
	'PATH_INFO',
	'SERVER_PROTOCOL',
	'SERVER_NAME',
	'SERVER_PORT',
	'REDIRECT_STATUS',
);

echo "<!DOCTYPE html>\n";
echo "<html lang=\"en\">\n";
echo "<head>\n";
echo "	<meta charset=\"UTF-8\">\n";
echo "	<title>PHP CGI test</title>\n";
echo "</head>\n";
echo "<body>\n";

echo "<h1>PHP CGI test page</h1>\n";
echo "<p>If you can read this page, the PHP CGI script was executed.</p>\n";

echo "<h2>Quick checks</h2>\n";
echo "<ul>\n";
echo "	<li>Current working directory: <strong>" . h(getcwd()) . "</strong></li>\n";
echo "	<li>Current date (server side): <strong>" . h(date('Y-m-d H:i:s')) . "</strong></li>\n";
echo "	<li>PHP version: <strong>" . h(PHP_VERSION) . "</strong></li>\n";
echo "	<li>Request method: <strong>" . h(isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : '') . "</strong></li>\n";
echo "</ul>\n";

echo "<h2>Self test form</h2>\n";
echo "<form method=\"POST\" action=\"\">\n";
echo "	<label>Name: <input type=\"text\" name=\"name\" value=\"webserv\"></label><br><br>\n";
echo "	<label>Message: <input type=\"text\" name=\"message\" value=\"hello from php-cgi\"></label><br><br>\n";
echo "	<input type=\"submit\" value=\"Send POST request\">\n";
echo "</form>\n";

echo "<p><a href=\"?hello=world&project=webserv\">Run a GET test</a></p>\n";

echo "<h2>Computed result</h2>\n";
if (isset($_POST['name']) || isset($_POST['message']) || !empty($_GET))
{
	$name = isset($_POST['name']) ? $_POST['name'] : 'anonymous';
	$message = isset($_POST['message']) ? $_POST['message'] : '';
	$hello = isset($_GET['hello']) ? $_GET['hello'] : '';
	$project = isset($_GET['project']) ? $_GET['project'] : '';

	echo "<p>";
	echo "Hello <strong>" . h($name) . "</strong>";
	if ($message !== '')
		echo ", message received: <strong>" . h($message) . "</strong>";
	if ($hello !== '' || $project !== '')
		echo ", GET values: hello=<strong>" . h($hello) . "</strong>, project=<strong>" . h($project) . "</strong>";
	echo ".</p>\n";

	echo "<p>Body length seen by PHP: <strong>" . h(strlen($raw_body)) . "</strong></p>\n";
}
else
{
	echo "<p>No GET or POST data received yet.</p>\n";
}

echo "<h2>Selected \$_SERVER values</h2>\n";
echo "<pre>";
foreach ($server_keys as $key)
{
	$value = isset($_SERVER[$key]) ? $_SERVER[$key] : '<absent>';
	echo h($key) . ' = ' . h($value) . "\n";
}
echo "</pre>";

print_array_html('$_GET', $_GET);
print_array_html('$_POST', $_POST);

echo "<h2>Raw request body</h2>\n";
if ($raw_body === '')
	echo "<p><em>empty</em></p>\n";
else
	echo "<pre>" . h($raw_body) . "</pre>\n";

echo "</body>\n";
echo "</html>\n";
