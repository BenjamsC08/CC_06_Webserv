import os
import sys

def main():
    print("Content-Type: text/plain")
    print()
    print("=== forbidden.py start ===")
    print("If you can read this, there is a problem (you should remove the reading permissions (`chmod -r ./CGI_scripts/err403_forbidden.py`))")
    print("=== forbidden.py end ===")
    return 0

if __name__ == "__main__":
    sys.exit(main())
