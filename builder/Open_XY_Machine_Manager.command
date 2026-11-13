#!/bin/bash
cd -- "$(dirname -- "$0")" || exit 1
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:$PATH"
for py in "$PWD/.venv/bin/python" /opt/homebrew/bin/python3.12 /opt/homebrew/bin/python3.13 /opt/homebrew/bin/python3 /usr/local/bin/python3; do
 if [ -x "$py" ] && "$py" -c 'import tkinter,sys;sys.exit(0 if sys.version_info>=(3,11) else 1)' 2>/dev/null; then
  "$py" app.py
  exit $?
 fi
done
echo 'Python 3.11+ with Tk is required. See README.md.'
read -r answer
exit 1
