import os
import sys

def main():
    print("Content-Type: text/plain")
    print("Set-Cookie: theme=light")
    print("Set-Cookie: ennemies=cat")
    print()
    print("=== script.py start ===")
    print("cwd =", os.getcwd())
    print("argc =", len(sys.argv))
    for i, arg in enumerate(sys.argv):
        print("argv[%d] = %r" % (i, arg))

    keys = [
        "GATEWAY_INTERFACE",
        "REQUEST_METHOD",
        "QUERY_STRING",
        "SERVER_PROTOCOL",
        "SERVER_PORT",
        "SERVER_NAME",
        "SERVER_SOFTWARE",
        "REMOTE_ADDR",
        "SCRIPT_NAME",
        "SCRIPT_FILENAME",
        "PATH_INFO",
        "REDIRECT_STATUS",
        "CONTENT_LENGTH",
        "CONTENT_TYPE",
    ]
    for key in keys:
        print("%s = %r" % (key, os.environ.get(key)))

    if "fail=yes" in os.environ.get("QUERY_STRING"):
        print("\nForced failure! An executioner rat has likely been corrupted \
by the Evil Giant Cat...\n")
        return 42

    print("=== script.py end ===")
    return 0

if __name__ == "__main__":
    sys.exit(main())
