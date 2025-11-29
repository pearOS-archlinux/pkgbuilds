#!/bin/bash

# Script to synchronize installed applications with PearID
# Uses API to send appIDs to server

# Configuration
# Configuration: API Base URL (can be overridden by PEARID_API_URL environment variable)
API_BASE_URL="${PEARID_API_URL:-https://account.pearos.xyz/api}"

# Load token storage helper
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/token_storage.sh" 2>/dev/null || {
    # Fallback if helper doesn't exist
    TOKEN_FILE="$HOME/.pearid_token"
    get_token() {
        if [ -f "$TOKEN_FILE" ] && [ -r "$TOKEN_FILE" ]; then
            cat "$TOKEN_FILE"
        elif [ -n "$PEARID_TOKEN" ]; then
            echo -n "$PEARID_TOKEN"
        fi
    }
}

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to display messages
print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

# Get token (from keychain/keyring, environment variable or file)
TOKEN=$(get_token)

if [ -z "$TOKEN" ]; then
    print_error "Token not found."
    print_info "Please authenticate first:"
    echo ""
    echo "Option 1 - Environment variable (most secure for scripts):"
    echo "  export PEARID_TOKEN=\$(curl -X POST $API_BASE_URL/login.php \\"
    echo "    -H 'Content-Type: application/json' \\"
    echo "    -d '{\"email\":\"your@email.com\",\"password\":\"yourpassword\"}' \\"
    echo "    | jq -r '.token')"
    echo ""
    echo "Option 2 - Use login script:"
    echo "  ./login_and_sync.sh your@email.com yourpassword"
    exit 1
fi

# Detect operating system
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS - use mdfind to find applications
    print_info "Detected macOS. Searching for installed applications..."
    
    # Find all .app applications
    APP_PATHS=$(mdfind "kMDItemKind == 'Application'" 2>/dev/null | grep "\.app$")
    
    APP_IDS=()
    while IFS= read -r app_path; do
        if [ -n "$app_path" ] && [ -d "$app_path" ]; then
            # Try to get Bundle Identifier from Info.plist (preferred method)
            app_id=$(defaults read "$app_path/Contents/Info.plist" CFBundleIdentifier 2>/dev/null)
            
            # If defaults read fails, try using mdls (metadata list)
            if [ -z "$app_id" ]; then
                app_id=$(mdls -name kMDItemCFBundleIdentifier -raw "$app_path" 2>/dev/null)
            fi
            
            # If still no Bundle ID, fallback to a generated one
            if [ -z "$app_id" ]; then
                app_name=$(basename "$app_path" .app)
                app_id="com.unknown.$(echo "$app_name" | tr '[:upper:]' '[:lower:]' | tr ' ' '.' | tr -d '[:punct:]')"
            fi
            
            # Only add non-empty app_id
            if [ -n "$app_id" ]; then
                APP_IDS+=("$app_id")
            fi
        fi
    done <<< "$APP_PATHS"
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux - use dpkg, pacman or flatpak
    print_info "Detected Linux. Searching for installed applications..."
    
    APP_IDS=()
    
    # Applications installed via pacman (Arch Linux)
    if command -v pacman &> /dev/null; then
        print_info "Detected pacman (Arch Linux). Listing packages..."
        while IFS= read -r package; do
            if [ -n "$package" ]; then
                app_id="com.archlinux.$(echo "$package" | tr '[:upper:]' '[:lower:]' | tr '_' '.')"
                APP_IDS+=("$app_id")
            fi
        done < <(pacman -Qq 2>/dev/null | head -50)
    fi
    
    # Applications installed via dpkg (Debian/Ubuntu)
    if command -v dpkg &> /dev/null; then
        print_info "Detected dpkg (Debian/Ubuntu). Listing packages..."
        while IFS= read -r package; do
            if [ -n "$package" ]; then
                app_id="com.debian.$(echo "$package" | cut -d: -f1)"
                APP_IDS+=("$app_id")
            fi
        done < <(dpkg-query -W -f='${Package}\n' 2>/dev/null | head -50)
    fi
    
    # Flatpak applications (available on all distributions)
    if command -v flatpak &> /dev/null; then
        print_info "Detected flatpak. Listing applications..."
        while IFS= read -r app; do
            if [ -n "$app" ]; then
                APP_IDS+=("$app")
            fi
        done < <(flatpak list --app --columns=application 2>/dev/null)
    fi
    
    # Snap applications (Ubuntu/Debian)
    if command -v snap &> /dev/null; then
        print_info "Detected snap. Listing applications..."
        while IFS= read -r app; do
            if [ -n "$app" ]; then
                app_id="com.snap.$(echo "$app" | tr '[:upper:]' '[:lower:]' | tr '_' '.')"
                APP_IDS+=("$app_id")
            fi
        done < <(snap list 2>/dev/null | tail -n +2 | awk '{print $1}')
    fi
else
    print_error "Unsupported operating system: $OSTYPE"
    exit 1
fi

if [ ${#APP_IDS[@]} -eq 0 ]; then
    print_error "No installed applications found"
    exit 1
fi

print_info "Found ${#APP_IDS[@]} applications. Sending to server..."

# Create JSON with appID list
DEVICE_NAME=$(hostname)
JSON_DATA=$(jq -n \
    --arg device "$DEVICE_NAME" \
    --argjson apps "$(printf '%s\n' "${APP_IDS[@]}" | jq -R . | jq -s .)" \
    '{device_name: $device, app_ids: $apps}')

# Send to API
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/apps.php" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d "$JSON_DATA")

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -eq 200 ]; then
    print_success "Applications synchronized successfully!"
    echo "$BODY" | jq '.'
else
    print_error "Synchronization error (HTTP $HTTP_CODE)"
    echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    
    if [ "$HTTP_CODE" -eq 401 ]; then
        print_info "Token expired. Please authenticate again."
        delete_token 2>/dev/null || true
    fi
    exit 1
fi

# Optional: List synchronized applications
print_info "Your synchronized applications:"
curl -s -X GET "$API_BASE_URL/apps.php" \
    -H "Authorization: Bearer $TOKEN" \
    | jq -r '.unique_apps[] | "  - \(.app_id) (on \(.device_count) devices)"'
