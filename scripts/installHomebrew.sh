#! /bin/bash
set -euo pipefail

# Set Architecture
ARCH=$(sysctl -n machdep.cpu.brand_string)

# Handle Command Line Arguments
while getopts "hu" ARG; do
  case "$ARG" in
    h) echo "USAGE: $(basename $0) -h, -u"
       exit 1 ;;
    # This is used for uninstalling if you already tried installing brew the normal way
    u) /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/uninstall.sh)" ;;
    :) echo "argument missing" >&2 ;;
    \?) echo "Something is wrong" >&2 ;;
  esac
done
shift "$((OPTIND-1))"


if [ "$ARCH" = "Apple M1" ]; then
  git config --global core.compression 0
  git config --global http.postBuffer 1048576000
fi

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

if [ "$ARCH" = "Apple M1" ]; then
  git config --global --unset core.compression
  git config --global --unset http.postBuffer
fi

echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc
eval "$(/opt/homebrew/bin/brew shellenv)"

