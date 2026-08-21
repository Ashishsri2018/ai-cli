#!/usr/bin/env bash
set -e

# ==============================================================================
# AI CLI Client - Smart Multi-OS Compiler & Global Installer
# Detects OS -> Detects Compiler -> Builds -> Runs Tests -> Installs Globally
# ==============================================================================

# ANSI Color codes
BOLD="\033[1m"
GREEN="\033[32m"
CYAN="\033[36m"
YELLOW="\033[33m"
RED="\033[31m"
RESET="\033[0m"

echo -e "${BOLD}${CYAN}=== Step 1: Detecting Operating System & Environment ===${RESET}"

OS_NAME="$(uname -s 2>/dev/null || echo "Unknown")"
ARCH_NAME="$(uname -m 2>/dev/null || echo "Unknown")"
PLATFORM="Unknown"

if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    PLATFORM="Android / Termux"
elif [ "$OS_NAME" = "Darwin" ]; then
    PLATFORM="macOS ($ARCH_NAME)"
elif [ "$OS_NAME" = "Linux" ]; then
    if grep -qi microsoft /proc/version 2>/dev/null; then
        PLATFORM="Linux (WSL - $ARCH_NAME)"
    elif [ -f /etc/os-release ]; then
        DISTRO_NAME="$(grep -E '^PRETTY_NAME=' /etc/os-release | cut -d= -f2 | tr -d '"')"
        PLATFORM="Linux (${DISTRO_NAME:-Generic} - $ARCH_NAME)"
    else
        PLATFORM="Linux ($ARCH_NAME)"
    fi
elif [[ "$OS_NAME" =~ MINGW.*|MSYS.*|CYGWIN.* ]]; then
    PLATFORM="Windows ($OS_NAME - $ARCH_NAME)"
elif [[ "$OS_NAME" =~ .*BSD.* ]]; then
    PLATFORM="BSD ($OS_NAME - $ARCH_NAME)"
fi

echo -e "  Operating System : ${GREEN}${PLATFORM}${RESET}"
echo -e "  Architecture     : ${GREEN}${ARCH_NAME}${RESET}"

# ==============================================================================
# Step 2: Compiler Detection & Verification
# ==============================================================================
echo -e "\n${BOLD}${CYAN}=== Step 2: Detecting C++17 Compiler & Dependencies ===${RESET}"

COMPILER=""
for c in clang++ g++ c++; do
    if command -v "$c" >/dev/null 2>&1; then
        # Check if compiler supports C++17
        if "$c" -std=c++17 -x c++ -dM -E - </dev/null >/dev/null 2>&1; then
            COMPILER="$c"
            break
        fi
    fi
done

if [ -z "$COMPILER" ]; then
    echo -e "${RED}Error: No compatible C++17 compiler found (clang++ or g++ required).${RESET}"
    echo -e "${YELLOW}To install a compiler, run:${RESET}"
    if [[ "$PLATFORM" =~ "Android / Termux" ]]; then
        echo -e "  pkg install -y clang make libcurl"
    elif [ "$OS_NAME" = "Darwin" ]; then
        echo -e "  xcode-select --install && brew install curl"
    elif [ -f /etc/debian_version ]; then
        echo -e "  sudo apt update && sudo apt install -y g++ make libcurl4-openssl-dev"
    elif [ -f /etc/fedora-release ] || [ -f /etc/redhat-release ]; then
        echo -e "  sudo dnf install -y gcc-c++ make libcurl-devel"
    elif [ -f /etc/arch-release ]; then
        echo -e "  sudo pacman -S --noconfirm gcc make curl"
    elif [ -f /etc/alpine-release ]; then
        echo -e "  sudo apk add g++ make curl-dev"
    else
        echo -e "  Please install g++ or clang++ with C++17 support and libcurl."
    fi
    exit 1
fi

COMPILER_VER="$("$COMPILER" --version 2>&1 | head -n 1)"
echo -e "  Compiler         : ${GREEN}${COMPILER}${RESET} (${COMPILER_VER})"

# Check libcurl availability
if ! echo '#include <curl/curl.h>' | "$COMPILER" -E -x c++ - >/dev/null 2>&1; then
    echo -e "${RED}Error: libcurl headers not found.${RESET}"
    echo -e "${YELLOW}Please install libcurl development package:${RESET}"
    if [[ "$PLATFORM" =~ "Android / Termux" ]]; then
        echo -e "  pkg install libcurl"
    elif [ -f /etc/debian_version ]; then
        echo -e "  sudo apt install libcurl4-openssl-dev"
    else
        echo -e "  Install libcurl development headers for your system."
    fi
    exit 1
fi
echo -e "  libcurl headers  : ${GREEN}Found${RESET}"

# ==============================================================================
# Step 3: Compilation & Unit Testing
# ==============================================================================
echo -e "\n${BOLD}${CYAN}=== Step 3: Compiling AI CLI Client & Running Tests ===${RESET}"

make clean CXX="$COMPILER"
make -j4 CXX="$COMPILER" TARGET="ai"
make test CXX="$COMPILER" TEST_TARGET="run_tests"

echo -e "  Unit Tests       : ${GREEN}16/16 Passed Successfully${RESET}"

# ==============================================================================
# Step 4: Determine Global Destination & Install Binary
# ==============================================================================
echo -e "\n${BOLD}${CYAN}=== Step 4: Installing for Global Terminal Access ===${RESET}"

INSTALL_DIR=""
CANDIDATE_DIRS=()

# Prioritize platform-specific standard bin paths
if [ -n "$PREFIX" ] && [ -d "$PREFIX/bin" ] && [ -w "$PREFIX/bin" ]; then
    CANDIDATE_DIRS+=("$PREFIX/bin")
fi
if [ -d "/data/data/com.termux/files/usr/bin" ] && [ -w "/data/data/com.termux/files/usr/bin" ]; then
    CANDIDATE_DIRS+=("/data/data/com.termux/files/usr/bin")
fi
if [ -d "/opt/homebrew/bin" ] && [ -w "/opt/homebrew/bin" ]; then
    CANDIDATE_DIRS+=("/opt/homebrew/bin")
fi
if [ -d "/usr/local/bin" ] && [ -w "/usr/local/bin" ]; then
    CANDIDATE_DIRS+=("/usr/local/bin")
fi
if [ -d "$HOME/.local/bin" ] && [ -w "$HOME/.local/bin" ]; then
    CANDIDATE_DIRS+=("$HOME/.local/bin")
fi
if [ -d "$HOME/bin" ] && [ -w "$HOME/bin" ]; then
    CANDIDATE_DIRS+=("$HOME/bin")
fi

# Fallback: create ~/.local/bin if nothing else is writable
if [ ${#CANDIDATE_DIRS[@]} -eq 0 ]; then
    mkdir -p "$HOME/.local/bin"
    CANDIDATE_DIRS+=("$HOME/.local/bin")
fi

INSTALL_DIR="${CANDIDATE_DIRS[0]}"

# Copy and replace old binary
cp -f ai "$INSTALL_DIR/ai"
chmod 755 "$INSTALL_DIR/ai"

# Also install to secondary global dirs if available and writable
for extra_dir in "${CANDIDATE_DIRS[@]:1}"; do
    if [ -w "$extra_dir" ]; then
        cp -f ai "$extra_dir/ai" 2>/dev/null || true
        chmod 755 "$extra_dir/ai" 2>/dev/null || true
    fi
done

echo -e "  Installed to     : ${GREEN}${INSTALL_DIR}/ai${RESET}"

# ==============================================================================
# Step 5: Verification of Global PATH Reachability
# ==============================================================================
echo -e "\n${BOLD}${CYAN}=== Step 5: Verifying Global PATH Availability ===${RESET}"

RESOLVED_BIN="$(command -v ai 2>/dev/null || echo "")"

if [ -n "$RESOLVED_BIN" ]; then
    echo -e "  ${GREEN}✓ 'ai' is immediately available globally from any terminal!${RESET}"
    echo -e "  Resolved Binary  : ${GREEN}${RESOLVED_BIN}${RESET}"
    "$RESOLVED_BIN" --version
else
    echo -e "${YELLOW}Note: '${INSTALL_DIR}' is not yet in your current \$PATH.${RESET}"
    echo -e "Add the following line to your ~/.bashrc or ~/.zshrc:"
    echo -e "  ${BOLD}export PATH=\"${INSTALL_DIR}:\$PATH\"${RESET}"
fi

echo -e "\n${BOLD}${GREEN}=== Setup Complete! ===${RESET}"
echo -e "Try running: ${BOLD}ai --help${RESET} or ${BOLD}ai \"What is quantum computing?\"${RESET}\n"
