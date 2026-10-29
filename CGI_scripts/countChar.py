#!/usr/bin/env python3

import os
import sys
import html
from urllib.parse import parse_qs


def get_request_body():
	content_length = os.environ.get("CONTENT_LENGTH", "")
	if not content_length:
		return ""
	try:
		length = int(content_length)
	except ValueError:
		return ""
	if length < 0:
		return ""
	return sys.stdin.read(length)


def get_input_text():
	method = os.environ.get("REQUEST_METHOD", "GET").upper()
	if method == "POST":
		body = get_request_body()
		content_type = os.environ.get("CONTENT_TYPE", "")
		if "application/x-www-form-urlencoded" in content_type:
			params = parse_qs(body, keep_blank_values=True)
			return params.get("text", [""])[0]
		return body
	return ""


def count_lines(text):
	if text == "":
		return 0
	return text.count("\n") + 1


def compute_stats(text):
	stats = {}

	stats["characters"] = len(text)
	stats["characters_no_newlines"] = len(text.replace("\n", "").replace("\r", ""))
	stats["utf8_bytes"] = len(text.encode("utf-8"))
	stats["lines"] = count_lines(text)
	stats["words"] = len(text.split())

	stats["spaces"] = text.count(" ")
	stats["tabs"] = text.count("\t")
	stats["newlines"] = text.count("\n")
	stats["carriage_returns"] = text.count("\r")

	stats["digits"] = sum(1 for c in text if c.isdigit())
	stats["letters"] = sum(1 for c in text if c.isalpha())
	stats["lowercase"] = sum(1 for c in text if c.islower())
	stats["uppercase"] = sum(1 for c in text if c.isupper())

	return stats


def print_html(text, stats):
	escaped_text = html.escape(text)

	sys.stdout.write("Content-Type: text/html; charset=UTF-8\r\n\r\n")

	sys.stdout.write("""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CGI Text Analyzer</title>
<style>
body {
	background: #56606f;
	color: #1f2328;
	font-family: Tahoma, Verdana, Arial, sans-serif;
	margin: 0;
	padding: 36px;
}

.window {
	max-width: 980px;
	margin: 0 auto;
	background: #aeb6c2;
	border: 2px solid;
	border-color: #d7dde5 #4a5260 #4a5260 #d7dde5;
	box-shadow: 8px 8px 0 rgba(20, 24, 30, 0.35);
}

.title-bar {
	background: linear-gradient(to right, #313a46, #667181);
	color: #f3f6fa;
	padding: 6px 8px;
	display: flex;
	justify-content: space-between;
	align-items: center;
	font-size: 13px;
	font-weight: bold;
	letter-spacing: 0.2px;
}

.title-bar-text {
	white-space: nowrap;
	overflow: hidden;
	text-overflow: ellipsis;
}

.title-bar-buttons {
	display: flex;
	gap: 3px;
	margin-left: 10px;
}

.title-button {
	width: 18px;
	height: 16px;
	background: #b8c0cb;
	border: 1px solid;
	border-color: #e1e6ed #4a5260 #4a5260 #e1e6ed;
	color: #20242a;
	display: flex;
	align-items: center;
	justify-content: center;
	font-size: 10px;
	line-height: 1;
	box-sizing: border-box;
}

.content {
	padding: 18px;
	background: #aeb6c2;
}

.panel {
	background: #bcc4cf;
	border: 2px solid;
	border-color: #dce2e8 #56606f #56606f #dce2e8;
	padding: 14px;
	margin-bottom: 16px;
}

h1 {
	margin: 0 0 10px 0;
	font-size: 28px;
	font-weight: bold;
	color: #1c2128;
}

h2 {
	margin: 0 0 12px 0;
	font-size: 20px;
	font-weight: bold;
	color: #1f252c;
}

.subtitle {
	margin: 0 0 14px 0;
	font-size: 14px;
	line-height: 1.5;
	color: #303741;
}

.note {
	margin-top: 14px;
	padding: 10px 12px;
	font-size: 13px;
	line-height: 1.5;
	color: #2a3139;
	background: #c7ced7;
	border: 1px solid;
	border-color: #e1e6ed #6d7785 #6d7785 #e1e6ed;
}

label {
	color: #232931;
	font-size: 14px;
}

textarea {
	width: 100%;
	min-height: 260px;
	font-family: "Courier New", monospace;
	font-size: 14px;
	line-height: 1.45;
	padding: 12px;
	box-sizing: border-box;
	resize: vertical;
	background: #eef2f6;
	color: #11161b;
	border: 2px solid;
	border-color: #5d6674 #e0e5eb #e0e5eb #5d6674;
	outline: none;
}

textarea:focus {
	background: #f6f8fb;
}

.button-row {
	margin-top: 14px;
}

button {
	padding: 8px 16px;
	font-size: 13px;
	font-family: Tahoma, Verdana, Arial, sans-serif;
	font-weight: bold;
	cursor: pointer;
	background: #b7bfca;
	color: #1b2026;
	border: 2px solid;
	border-color: #e1e6ed #55606e #55606e #e1e6ed;
	min-width: 130px;
}

button:active {
	border-color: #55606e #e1e6ed #e1e6ed #55606e;
	padding: 9px 15px 7px 17px;
}

table {
	width: 100%;
	border-collapse: collapse;
	background: #c3cad4;
	border: 2px solid;
	border-color: #dfe4ea #596270 #596270 #dfe4ea;
}

th, td {
	padding: 10px 12px;
	text-align: left;
	border: 1px solid #7b8593;
}

th {
	background: #8e99a8;
	color: #11161b;
	font-weight: bold;
}

td {
	background: #d5dbe3;
	color: #1c2127;
}

tr:nth-child(even) td {
	background: #cbd2db;
}

.preview {
	margin-top: 16px;
	padding: 12px;
	background: #eef2f6;
	color: #14191f;
	border: 2px solid;
	border-color: #5d6674 #e0e5eb #e0e5eb #5d6674;
	white-space: pre-wrap;
	font-family: "Courier New", monospace;
	line-height: 1.45;
}

.footer-note {
	margin-top: 14px;
	padding: 10px 12px;
	font-size: 13px;
	line-height: 1.5;
	color: #2f363f;
	background: #c7ced7;
	border: 1px solid;
	border-color: #e1e6ed #6d7785 #6d7785 #e1e6ed;
}

strong {
	color: #151a20;
}

.metric-name {
	width: 65%;
}

.metric-value {
	width: 35%;
}
</style>
</head>
<body>
<div class="window">
	<div class="title-bar">
		<div class="title-bar-text">CGI Text Analyzer</div>
		<div class="title-bar-buttons">
			<div class="title-button">_</div>
			<div class="title-button">□</div>
			<div class="title-button">×</div>
		</div>
	</div>

	<div class="content">
		<div class="panel">
			<h1>CGI Text Analyzer</h1>
			<p class="subtitle">Enter some text, submit it with POST, then review the statistics computed by the CGI script.</p>
			<p class="note">If clicking on "Analyze text" sends you to a 405 error page, make sure the 'POST' method is explicitly allowed for this location in the configuration file.</p>
		</div>

		<div class="panel">
			<form method="POST" action="">
				<label for="text"><strong>Text to analyze:</strong></label><br><br>
""")

	sys.stdout.write('<textarea id="text" name="text">')
	sys.stdout.write(escaped_text)
	sys.stdout.write("""</textarea>
				<div class="button-row">
					<button type="submit">Analyze text</button>
				</div>
			</form>
		</div>""")

	if stats is not None:
		sys.stdout.write("""
		<div class="panel">
			<h2>Results</h2>
			<table>
				<tr><th class="metric-name">Metric</th><th class="metric-value">Value</th></tr>
				<tr><td>Number of characters</td><td>%d</td></tr>
				<tr><td>Number of characters excluding line breaks</td><td>%d</td></tr>
				<tr><td>Number of UTF-8 bytes</td><td>%d</td></tr>
				<tr><td>Number of lines</td><td>%d</td></tr>
				<tr><td>Number of words</td><td>%d</td></tr>
				<tr><td>Number of spaces</td><td>%d</td></tr>
				<tr><td>Number of tabs</td><td>%d</td></tr>
				<tr><td>Number of \\n</td><td>%d</td></tr>
				<tr><td>Number of \\r</td><td>%d</td></tr>
				<tr><td>Number of digits</td><td>%d</td></tr>
				<tr><td>Number of letters</td><td>%d</td></tr>
				<tr><td>Number of lowercase letters</td><td>%d</td></tr>
				<tr><td>Number of uppercase letters</td><td>%d</td></tr>
			</table>

			<div class="preview"><strong>Preview of the received text:</strong>

%s</div>

			<div class="footer-note">
Rule used here: if the text is empty, the line count is 0.
Otherwise, the line count is the number of '\\n' characters plus 1.
			</div>
		</div>
""" % (
			stats["characters"],
			stats["characters_no_newlines"],
			stats["utf8_bytes"],
			stats["lines"],
			stats["words"],
			stats["spaces"],
			stats["tabs"],
			stats["newlines"],
			stats["carriage_returns"],
			stats["digits"],
			stats["letters"],
			stats["lowercase"],
			stats["uppercase"],
			escaped_text if escaped_text else "<em>(empty)</em>"
		))

	sys.stdout.write("""
	</div>
</div>
</body>
</html>""")


def main():
	text = get_input_text()
	stats = None

	if os.environ.get("REQUEST_METHOD", "GET").upper() == "POST":
		stats = compute_stats(text)

	print_html(text, stats)


if __name__ == "__main__":
	main()
