#!/bin/bash

# Script to restore user settings from PearID backup
# Usage: ./restore_settings.sh --conf-path "/path/to/settings" [--device-id "device_id"]

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
BLUE='\033[0;34m'
NC='\033[0m'

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

print_data() {
    echo -e "${BLUE}→${NC} $1"
}

# Usage function
show_usage() {
    echo "Usage: $0 [--list] [\"/path/to/settings\"] [options]"
    echo ""
    echo "Options:"
    echo "  --list                             List all available files for restore"
    echo "  --device-id \"device_id\"           Optional: Filter by specific device (SHA256 hash)"
    echo "  --help                             Show this message"
    echo ""
    echo "Arguments:"
    echo "  \"/path/to/settings\"               Path to settings to restore (e.g., \"/var/run/tempfile.txt\")"
    echo "                                     Required if --list is not used"
    echo ""
    echo "Examples:"
    echo "  $0 --list                          List all backed up files"
    echo "  $0 --list --device-id \"abc123...\"  List files for specific device"
    echo "  $0 \"/var/run/tempfile.txt\"        Restore specific file"
    echo "  $0 \"/var/run/\"                    Restore directory settings"
    echo ""
    echo "Note: Without --list, this script outputs the raw content of settings."
}

# Get token
TOKEN=$(get_token)

if [ -z "$TOKEN" ]; then
    print_error "Token not found."
    print_info "Please authenticate first:"
    echo ""
    echo "  ./login_and_sync.sh your@email.com yourpassword"
    exit 1
fi

# Parse arguments
LIST_MODE=false
CONF_PATH=""
DEVICE_ID=""

# Parse options and arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --list)
            LIST_MODE=true
            shift
            ;;
        --device-id)
            DEVICE_ID="$2"
            shift 2
            ;;
        --help)
            show_usage
            exit 0
            ;;
        -*)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
        *)
            # This is a path argument
            if [ -z "$CONF_PATH" ]; then
                CONF_PATH="$1"
            else
                print_error "Multiple path arguments provided. Use --list to see all files or provide a single path."
                show_usage
                exit 1
            fi
            shift
            ;;
    esac
done

# Validate arguments based on mode
if [ "$LIST_MODE" = false ] && [ -z "$CONF_PATH" ]; then
    print_error "Either --list or a path argument is required"
    show_usage
    exit 1
fi

# Generate device_id if not provided and device_id.sh exists (only if not in list mode or if device_id needed)
if [ -z "$DEVICE_ID" ] && [ -f "$SCRIPT_DIR/device_id.sh" ] && [ "$LIST_MODE" = false ]; then
    DEVICE_ID=$(bash "$SCRIPT_DIR/device_id.sh" 2>/dev/null)
fi

# Handle --list mode
if [ "$LIST_MODE" = true ]; then
    # Build query parameters for API (list all)
    QUERY_PARAMS=""
    
    if [ -n "$DEVICE_ID" ]; then
        if [[ ! "$DEVICE_ID" =~ ^[a-f0-9]{64}$ ]]; then
            print_error "Invalid device_id format. Must be a 64-character hexadecimal string (SHA256 hash)"
            exit 1
        fi
        QUERY_PARAMS="?device_id=$DEVICE_ID"
    fi
    
    # Fetch all settings from API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/settings.php$QUERY_PARAMS" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" 2>/dev/null)
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -ne 200 ]; then
        echo "Error fetching settings (HTTP $HTTP_CODE)" >&2
        echo "$BODY" | jq '.' 2>/dev/null >&2 || echo "$BODY" >&2
        exit 1
    fi
    
    # Parse and display settings
    if ! command -v jq &> /dev/null; then
        echo "jq is required for this script. Please install jq first." >&2
        exit 1
    fi
    
    # Process each setting and display in the requested format
    echo "$BODY" | jq -r '.settings[] | 
        (.conf_path + .conf_name) as $path |
        (.last_backup // "Never" | split(" ") | .[0]) as $date |
        "\($path) * Last Backup: \($date)"' 2>/dev/null
    
    exit 0
fi

# Convert conf_path to absolute path
ABS_CONF_PATH=""
if [[ "$CONF_PATH" = /* ]]; then
    # Already absolute path
    ABS_CONF_PATH="$CONF_PATH"
else
    # Relative path, convert to absolute
    if [[ "$CONF_PATH" =~ ^\./ ]] || [[ "$CONF_PATH" != */* ]]; then
        # Path starting with ./ or no directory (just filename)
        ABS_CONF_PATH=$(cd "$(dirname "$CONF_PATH" 2>/dev/null || echo .)" && pwd)/$(basename "$CONF_PATH")
    else
        # Relative path with directory
        ABS_CONF_PATH=$(cd "$(dirname "$CONF_PATH")" 2>/dev/null && pwd)/$(basename "$CONF_PATH")
    fi
fi

# Extract directory path from absolute conf_path (if it's a file path, get directory; if directory, ensure trailing slash)
SEARCH_CONF_PATH=$(dirname "$ABS_CONF_PATH")
# Add trailing slash if directory path doesn't end with one
if [ "$SEARCH_CONF_PATH" != "/" ] && [ "${SEARCH_CONF_PATH: -1}" != "/" ]; then
    SEARCH_CONF_PATH="${SEARCH_CONF_PATH}/"
fi

# Extract filename if conf_path looks like a file path
SEARCH_CONF_NAME=""
if [ "$ABS_CONF_PATH" != "$SEARCH_CONF_PATH" ]; then
    # It's a file path, extract filename
    SEARCH_CONF_NAME=$(basename "$ABS_CONF_PATH")
fi

# Build query parameters for API
QUERY_PARAMS="?conf_path=$(printf '%s' "$SEARCH_CONF_PATH" | jq -sRr @uri)"

if [ -n "$DEVICE_ID" ]; then
    if [[ ! "$DEVICE_ID" =~ ^[a-f0-9]{64}$ ]]; then
        print_error "Invalid device_id format. Must be a 64-character hexadecimal string (SHA256 hash)"
        exit 1
    fi
    QUERY_PARAMS="${QUERY_PARAMS}&device_id=$DEVICE_ID"
fi

# Note: conf_name filtering will be done after fetching if SEARCH_CONF_NAME is set

# Fetch settings from API (silently, no output)
RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/settings.php$QUERY_PARAMS" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" 2>/dev/null)

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -ne 200 ]; then
    # Errors still go to stderr
    echo "Error fetching settings (HTTP $HTTP_CODE)" >&2
    echo "$BODY" | jq '.' 2>/dev/null >&2 || echo "$BODY" >&2
    exit 1
fi

# Parse settings
if ! command -v jq &> /dev/null; then
    echo "jq is required for this script. Please install jq first." >&2
    exit 1
fi

# Filter settings by conf_name if searching for a specific file
if [ -n "$SEARCH_CONF_NAME" ]; then
    SETTINGS_JSON=$(echo "$BODY" | jq -c ".settings[] | select(.conf_name == \"$SEARCH_CONF_NAME\")" 2>/dev/null)
else
    SETTINGS_JSON=$(echo "$BODY" | jq -c '.settings[]' 2>/dev/null)
fi

# Count filtered settings (count non-empty lines)
if [ -z "$SETTINGS_JSON" ]; then
    SETTINGS_COUNT_ACTUAL=0
else
    SETTINGS_COUNT_ACTUAL=$(echo "$SETTINGS_JSON" | grep -c . 2>/dev/null || echo "0")
    # Ensure it's a number by removing any whitespace
    SETTINGS_COUNT_ACTUAL=$(echo "$SETTINGS_COUNT_ACTUAL" | tr -d '[:space:]' | head -n1)
    # Default to 0 if empty or not a number
    if [ -z "$SETTINGS_COUNT_ACTUAL" ] || ! [[ "$SETTINGS_COUNT_ACTUAL" =~ ^[0-9]+$ ]]; then
        SETTINGS_COUNT_ACTUAL=0
    fi
fi

# Convert to integer for comparison
SETTINGS_COUNT_ACTUAL=$((SETTINGS_COUNT_ACTUAL + 0))

if [ "$SETTINGS_COUNT_ACTUAL" -eq 0 ]; then
    # No settings found - return "not_backed_up"
    echo "not_backed_up"
    exit 0
fi

# Process and display settings
# If there's only one setting and it's a raw content (conf_name matches filename), display raw content
# Otherwise, display as KEY=VALUE pairs or raw content depending on format

# Check if all settings have the same conf_path and if there's a single setting with a filename-like conf_name
FIRST_SETTING=$(echo "$SETTINGS_JSON" | head -n1)
if [ -z "$FIRST_SETTING" ]; then
    echo "not_backed_up"
    exit 0
fi

FIRST_CONF_NAME=$(echo "$FIRST_SETTING" | jq -r '.conf_name')

# Check if this looks like a raw content file (conf_name matches a filename pattern)
IS_RAW_CONTENT=false
if echo "$FIRST_CONF_NAME" | grep -q '\.'; then
    # Has extension, likely a filename
    IS_RAW_CONTENT=true
fi

if [ "$SETTINGS_COUNT_ACTUAL" -eq 1 ] && [ "$IS_RAW_CONTENT" = true ]; then
    # Single raw content file - display raw content only
    RAW_CONTENT=$(echo "$FIRST_SETTING" | jq -r '.conf_value')
    echo "$RAW_CONTENT"
else
    # Multiple settings or KEY=VALUE format - display formatted
    while IFS= read -r setting; do
        if [ -n "$setting" ]; then
            conf_name=$(echo "$setting" | jq -r '.conf_name')
            conf_value=$(echo "$setting" | jq -r '.conf_value')
            
            # Check if value contains newlines (raw content)
            if echo "$conf_value" | grep -q $'\n'; then
                # Raw content - display with separator
                if [ "$SETTINGS_COUNT_ACTUAL" -gt 1 ]; then
                    echo "--- Setting: $conf_name ---"
                fi
                echo "$conf_value"
                if [ "$SETTINGS_COUNT_ACTUAL" -gt 1 ]; then
                    echo ""
                fi
            else
                # KEY=VALUE format
                echo "${conf_name}=${conf_value}"
            fi
        fi
    done <<< "$SETTINGS_JSON"
fi

