import os
import sys

body = sys.stdin.read()

print("Content-Type: text/plain")
print()
print("=== CGI ENV ===")
for key in [
	"REQUEST_METHOD",
	"CONTENT_TYPE",
	"CONTENT_LENGTH",
	"QUERY_STRING",
	"SCRIPT_NAME",
	"PATH_INFO",
	"SERVER_PROTOCOL",
	"SERVER_NAME",
	"SERVER_PORT",
]:
	print(f"{key}={os.environ.get(key, '')}")

print()
print("=== STDIN ===")
print("```")
print(body)
print("```")
