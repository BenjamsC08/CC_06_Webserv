import os
import sys


def write_stdout(text):
	sys.stdout.write(text)
	sys.stdout.flush()


def is_probably_cgi():
	return (
		"GATEWAY_INTERFACE" in os.environ
		or "REQUEST_METHOD" in os.environ
		or "SERVER_PROTOCOL" in os.environ
	)


def main():
	cwd = os.getcwd()
	stdin_data = sys.stdin.read()

	env_keys = [
		"GATEWAY_INTERFACE",
		"REQUEST_METHOD",
		"QUERY_STRING",
		"CONTENT_LENGTH",
		"CONTENT_TYPE",
		"SCRIPT_NAME",
		"PATH_INFO",
		"SERVER_PROTOCOL",
		"SERVER_NAME",
		"SERVER_PORT",
	]

	if is_probably_cgi():
		write_stdout("Content-Type: text/plain\r\n\r\n")

	write_stdout("=== CWD ===\n")
	write_stdout(cwd + "\n\n")

	write_stdout("=== ENV ===\n")
	for key in env_keys:
		value = os.environ.get(key, "<absent>")
		write_stdout(key + "=" + value + "\n")

	write_stdout("\n=== STDIN ===\n")
	write_stdout(stdin_data)


if __name__ == "__main__":
	main()
