#!/bin/bash

# Script to backup user settings to PearID
# Usage: ./backup_settings.sh "/path/to/file" [--device-id "device_id"]

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
    echo "Usage: $0 \"/path/to/file\" [options]"
    echo ""
    echo "Arguments:"
    echo "  \"/path/to/file\"                  REQUIRED: Path to file to backup"
    echo ""
    echo "Options:"
    echo "  --device-id \"device_id\"           Optional: Link settings to a specific device (SHA256 hash)"
    echo "  --help                             Show this message"
    echo ""
    echo "Examples:"
    echo "  $0 \"/path/to/config.conf\""
    echo "  $0 \"./test.cfg\""
    echo "  $0 \"/home/user/.config/myapp/config.conf\" --device-id \"abc123...\""
    echo ""
    echo "Note: This script backs up the raw content of a single file."
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
FILE_PATH=""
DEVICE_ID=""

# First argument is the file path (required)
if [ $# -gt 0 ] && [[ "$1" != --* ]]; then
    FILE_PATH="$1"
    shift
fi

# Parse remaining options
while [[ $# -gt 0 ]]; do
    case $1 in
        --device-id)
            DEVICE_ID="$2"
            shift 2
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate required argument
if [ -z "$FILE_PATH" ]; then
    print_error "File path is required"
    show_usage
    exit 1
fi

# Generate device_id if not provided and device_id.sh exists
if [ -z "$DEVICE_ID" ] && [ -f "$SCRIPT_DIR/device_id.sh" ]; then
    DEVICE_ID=$(bash "$SCRIPT_DIR/device_id.sh" 2>/dev/null)
fi

# Function to read setting from file
read_setting_from_file() {
    local file_path="$1"
    local conf_path="$2"
    
    if [ ! -f "$file_path" ]; then
        return 1
    fi
    
    # Try different config formats
    # Format 1: KEY=VALUE (shell config)
    # Format 2: KEY: VALUE (YAML-like)
    # Format 3: KEY VALUE (simple)
    # Format 4: JSON format
    
    local settings=()
    
    # Check if it's a JSON file
    if command -v jq &> /dev/null && jq -e . "$file_path" >/dev/null 2>&1; then
        # JSON format
        while IFS='=' read -r key value; do
            if [ -n "$key" ] && [ -n "$value" ]; then
                settings+=("{\"conf_path\":\"$conf_path\",\"conf_name\":\"$key\",\"conf_value\":$value}")
            fi
        done < <(jq -r 'to_entries[] | "\(.key)=\(.value | tostring)"' "$file_path" 2>/dev/null)
    else
        # Read as text file
        while IFS= read -r line || [ -n "$line" ]; do
            # Skip comments and empty lines
            if [[ "$line" =~ ^[[:space:]]*# ]] || [[ -z "${line// }" ]]; then
                continue
            fi
            
            # Try different formats
            if [[ "$line" =~ ^[[:space:]]*([^=[:space:]]+)=(.*)$ ]]; then
                # KEY=VALUE format
                key="${BASH_REMATCH[1]// /}"
                value="${BASH_REMATCH[2]// /}"
                # Remove quotes if present
                value="${value#\"}"
                value="${value%\"}"
                value="${value#\'}"
                value="${value%\'}"
                if [ -n "$key" ] && [ -n "$value" ]; then
                    key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                    value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                    settings+=("{\"conf_path\":\"$conf_path\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                fi
            elif [[ "$line" =~ ^[[:space:]]*([^:[:space:]]+):[[:space:]]*(.*)$ ]]; then
                # KEY: VALUE format
                key="${BASH_REMATCH[1]// /}"
                value="${BASH_REMATCH[2]// /}"
                value="${value#\"}"
                value="${value%\"}"
                value="${value#\'}"
                value="${value%\'}"
                if [ -n "$key" ] && [ -n "$value" ]; then
                    key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                    value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                    settings+=("{\"conf_path\":\"$conf_path\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                fi
            elif [[ "$line" =~ ^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+.*)$ ]]; then
                # KEY VALUE format
                key="${BASH_REMATCH[1]// /}"
                value="${BASH_REMATCH[2]// /}"
                value="${value#\"}"
                value="${value%\"}"
                value="${value#\'}"
                value="${value%\'}"
                if [ -n "$key" ] && [ -n "$value" ]; then
                    key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                    value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                    settings+=("{\"conf_path\":\"$conf_path\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                fi
            fi
        done < "$file_path"
    fi
    
    printf '%s\n' "${settings[@]}"
}

# Function to backup directory
backup_directory() {
    local dir_path="$1"
    local device_id="$2"
    
    if [ ! -d "$dir_path" ]; then
        print_error "Directory does not exist: $dir_path"
        return 1
    fi
    
    print_info "Scanning directory: $dir_path"
    
    local settings_array=()
    local count=0
    
    # Find all config files
    while IFS= read -r -d '' file; do
        print_data "Reading: $file"
        local file_settings
        file_settings=$(read_setting_from_file "$file" "$dir_path")
        
        if [ -n "$file_settings" ]; then
            while IFS= read -r setting; do
                if [ -n "$setting" ]; then
                    settings_array+=("$setting")
                    ((count++))
                fi
            done <<< "$file_settings"
        fi
    done < <(find "$dir_path" -type f \( -name "*.conf" -o -name "*.config" -o -name "*.cfg" -o -name "*.ini" -o -name "*.json" -o -name "*rc" -o -name "*.properties" \) -print0 2>/dev/null)
    
    if [ ${#settings_array[@]} -eq 0 ]; then
        print_error "No settings found in directory: $dir_path"
        return 1
    fi
    
    print_info "Found $count settings. Uploading to server..."
    
    # Build JSON
    local json_settings="["
    local first=true
    for setting in "${settings_array[@]}"; do
        if [ "$first" = true ]; then
            first=false
        else
            json_settings+=","
        fi
        json_settings+="$setting"
    done
    json_settings+="]"
    
    # Build full JSON with optional device_id
    local full_json="{"
    if [ -n "$device_id" ]; then
        full_json+="\"device_id\":\"$device_id\","
    fi
    full_json+="\"settings\":$json_settings}"
    
    # Send to API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/settings.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$full_json")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Settings backed up successfully!"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    else
        print_error "Error backing up settings (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
}

# Function to backup single file
backup_file() {
    local file_path="$1"
    local device_id="$2"
    
    if [ ! -f "$file_path" ]; then
        print_error "File does not exist: $file_path"
        exit 1
    fi
    
    # Get absolute directory path and ensure it ends with a slash
    local abs_file_path
    if [[ "$file_path" = /* ]]; then
        # Already absolute path
        abs_file_path="$file_path"
    else
        # Relative path, convert to absolute
        abs_file_path=$(cd "$(dirname "$file_path")" && pwd)/$(basename "$file_path")
    fi
    
    local conf_dir=$(dirname "$abs_file_path")
    # Add trailing slash if directory path doesn't end with one
    if [ "$conf_dir" != "/" ] && [ "${conf_dir: -1}" != "/" ]; then
        conf_dir="${conf_dir}/"
    fi
    
    # Get filename as conf_name
    local filename=$(basename "$file_path")
    
    # Check file size before reading (max 2048KB = 2MB)
    local max_size_bytes=$((2048 * 1024))
    local file_size
    file_size=$(stat -f%z "$file_path" 2>/dev/null || stat -c%s "$file_path" 2>/dev/null || echo "0")
    
    if [ "$file_size" -gt "$max_size_bytes" ]; then
        print_error "File size exceeds maximum allowed size (2MB)"
        echo "file_over_2mb"
        exit 1
    fi
    
    print_info "Reading raw content from file: $file_path"
    
    # Read raw content from file
    local raw_content
    raw_content=$(cat "$file_path")
    
    if [ -z "$raw_content" ]; then
        # Empty file is still valid, just warn
        print_info "File is empty, backing up empty content"
    fi
    
    # Check if jq is available (required)
    if ! command -v jq &> /dev/null; then
        print_error "jq is required for backing up settings. Please install jq first."
        exit 1
    fi
    
    # Build setting JSON using jq
    local setting_json
    setting_json=$(jq -n \
        --arg conf_path "$conf_dir" \
        --arg conf_name "$filename" \
        --arg conf_value "$raw_content" \
        '{conf_path: $conf_path, conf_name: $conf_name, conf_value: $conf_value}')
    
    # Build settings array JSON
    local settings_json_array
    settings_json_array=$(echo "$setting_json" | jq -s '.')
    
    # Build full JSON with jq
    if [ -n "$device_id" ]; then
        full_json=$(jq -n \
            --argjson settings "$settings_json_array" \
            --arg device_id "$device_id" \
            '{device_id: $device_id, settings: $settings}')
    else
        full_json=$(jq -n \
            --argjson settings "$settings_json_array" \
            '{settings: $settings}')
    fi
    
    # Verify final JSON is valid
    if [ -z "$full_json" ] || [ "$full_json" = "null" ]; then
        print_error "Failed to build final JSON"
        exit 1
    fi
    
    print_info "Backing up file as raw content. Uploading to server..."
    
    # Send to API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/settings.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$full_json")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Settings backed up successfully!"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    else
        # Check if error is file_over_2mb
        ERROR_TYPE=$(echo "$BODY" | jq -r '.error' 2>/dev/null)
        if [ "$ERROR_TYPE" = "file_over_2mb" ]; then
            print_error "File size exceeds maximum allowed size (2MB)"
            echo "file_over_2mb"
        else
            print_error "Error backing up settings (HTTP $HTTP_CODE)"
            echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        fi
        exit 1
    fi
}

# Function to backup manual content
backup_manual_content() {
    local conf_path="$1"
    local content="$2"
    local device_id="$3"
    
    if [ -z "$conf_path" ] || [ -z "$content" ]; then
        print_error "conf_path and content are required for manual backup"
        return 1
    fi
    
    # Get directory path and ensure it ends with a slash
    local conf_dir=$(dirname "$conf_path")
    # Add trailing slash if directory path doesn't end with one
    if [ "$conf_dir" != "/" ] && [ "${conf_dir: -1}" != "/" ]; then
        conf_dir="${conf_dir}/"
    fi
    
    print_info "Backing up manual content to path: $conf_path"
    
    # Parse content and extract settings
    local settings_array=()
    local count=0
    
    # Process literal \n to actual newlines if present (before parsing)
    local processed_content="$content"
    # Check if content contains literal backslash-n (not actual newline)
    # When passed via command line, \n is literal, so convert it
    if [[ "$content" == *"\\n"* ]] && [[ "$content" != *$'\n'* ]]; then
        # Replace literal \n with actual newlines using printf %b
        processed_content=$(printf '%b' "$content")
    fi
    
    # Try to parse content line by line
    while IFS= read -r line || [ -n "$line" ]; do
        # Skip comments and empty lines
        if [[ "$line" =~ ^[[:space:]]*# ]] || [[ -z "${line// }" ]]; then
            continue
        fi
        
        # Try different formats
        if [[ "$line" =~ ^[[:space:]]*\[.*\]$ ]]; then
            # Section header, skip for now
            continue
        elif [[ "$line" =~ ^[[:space:]]*([^=[:space:]]+)=(.*)$ ]]; then
            # KEY=VALUE format
            key="${BASH_REMATCH[1]// /}"
            value="${BASH_REMATCH[2]// /}"
            # Remove quotes if present
            value="${value#\"}"
            value="${value%\"}"
            value="${value#\'}"
            value="${value%\'}"
            if [ -n "$key" ] && [ -n "$value" ]; then
                key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                settings_array+=("{\"conf_path\":\"$conf_dir\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                ((count++))
            fi
        elif [[ "$line" =~ ^[[:space:]]*([^:[:space:]]+):[[:space:]]*(.*)$ ]]; then
            # KEY: VALUE format
            key="${BASH_REMATCH[1]// /}"
            value="${BASH_REMATCH[2]// /}"
            value="${value#\"}"
            value="${value%\"}"
            value="${value#\'}"
            value="${value%\'}"
            if [ -n "$key" ] && [ -n "$value" ]; then
                key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                settings_array+=("{\"conf_path\":\"$conf_dir\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                ((count++))
            fi
        elif [[ "$line" =~ ^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+.*)$ ]]; then
            # KEY VALUE format
            key="${BASH_REMATCH[1]// /}"
            value="${BASH_REMATCH[2]// /}"
            value="${value#\"}"
            value="${value%\"}"
            value="${value#\'}"
            value="${value%\'}"
            if [ -n "$key" ] && [ -n "$value" ]; then
                key_escaped=$(echo "$key" | sed 's/"/\\"/g')
                value_escaped=$(echo "$value" | sed 's/"/\\"/g')
                settings_array+=("{\"conf_path\":\"$conf_dir\",\"conf_name\":\"$key_escaped\",\"conf_value\":\"$value_escaped\"}")
                ((count++))
            fi
        fi
    done <<< "$processed_content"
    
    # If no settings were parsed, save the entire content as a single setting
    if [ ${#settings_array[@]} -eq 0 ]; then
        print_info "Content doesn't match standard format, saving as raw content"
        local filename=$(basename "$conf_path")
        # Use full filename as conf_name (including extension)
        local conf_name="$filename"
        if [ -z "$conf_name" ]; then
            conf_name="content"
        fi
        
        # Ensure conf_dir has trailing slash (already set above)
        
        # Use jq to build proper JSON (required for proper escaping)
        if command -v jq &> /dev/null; then
            # Use jq to build proper JSON
            local setting_json
            setting_json=$(jq -n \
                --arg conf_path "$conf_dir" \
                --arg conf_name "$conf_name" \
                --arg conf_value "$processed_content" \
                '{conf_path: $conf_path, conf_name: $conf_name, conf_value: $conf_value}')
            settings_array+=("$setting_json")
        else
            print_error "jq is required for backing up settings. Please install jq first."
            exit 1
        fi
        count=1
    fi
    
    if [ $count -eq 0 ]; then
        print_error "No settings found in content"
        return 1
    fi
    
    print_info "Found $count setting(s). Uploading to server..."
    
    # Build JSON using jq (required for proper escaping)
    if ! command -v jq &> /dev/null; then
        print_error "jq is required for backing up settings. Please install jq first."
        exit 1
    fi
    
    # Combine all settings into a JSON array using jq (slurp)
    local settings_json_array
    settings_json_array=$(printf '%s\n' "${settings_array[@]}" | jq -s '.')
    
    # Validate the array before building final JSON
    if [ -z "$settings_json_array" ] || [ "$settings_json_array" = "null" ]; then
        print_error "Failed to build settings array"
        exit 1
    fi
    
    # Build full JSON with jq
    if [ -n "$device_id" ]; then
        full_json=$(jq -n \
            --argjson settings "$settings_json_array" \
            --arg device_id "$device_id" \
            '{device_id: $device_id, settings: $settings}')
    else
        full_json=$(jq -n \
            --argjson settings "$settings_json_array" \
            '{settings: $settings}')
    fi
    
    # Verify final JSON is valid
    if [ -z "$full_json" ] || [ "$full_json" = "null" ]; then
        print_error "Failed to build final JSON"
        exit 1
    fi
    
    # Validate JSON before sending
    if command -v jq &> /dev/null; then
        if ! echo "$full_json" | jq . >/dev/null 2>&1; then
            print_error "Generated JSON is invalid"
            echo "JSON: $full_json" >&2
            exit 1
        fi
    fi
    
    # Debug: print JSON being sent (first 500 chars)
    if [ -n "$DEBUG" ]; then
        echo "JSON being sent: ${full_json:0:500}..." >&2
    fi
    
    # Send to API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/settings.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$full_json")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Settings backed up successfully!"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    else
        print_error "Error backing up settings (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
}

# Function to backup single setting
backup_single_setting() {
    local conf_path="$1"
    local conf_name="$2"
    local conf_value="$3"
    local device_id="$4"
    
    if [ -z "$conf_path" ] || [ -z "$conf_name" ] || [ -z "$conf_value" ]; then
        print_error "conf_path, conf_name, and conf_value are required"
        return 1
    fi
    
    # Get directory path and ensure it ends with a slash
    local conf_dir=$(dirname "$conf_path")
    # Add trailing slash if directory path doesn't end with one
    if [ "$conf_dir" != "/" ] && [ "${conf_dir: -1}" != "/" ]; then
        conf_dir="${conf_dir}/"
    fi
    
    print_info "Backing up setting: $conf_name = $conf_value"
    print_info "Path: $conf_dir"
    
    # Escape values for JSON
    conf_name_escaped=$(echo "$conf_name" | sed 's/"/\\"/g')
    conf_value_escaped=$(echo "$conf_value" | sed 's/"/\\"/g')
    
    # Build JSON
    local json_settings="[{\"conf_path\":\"$conf_dir\",\"conf_name\":\"$conf_name_escaped\",\"conf_value\":\"$conf_value_escaped\"}]"
    
    # Build full JSON with optional device_id
    local full_json="{"
    if [ -n "$device_id" ]; then
        full_json+="\"device_id\":\"$device_id\","
    fi
    full_json+="\"settings\":$json_settings}"
    
    # Send to API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/settings.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$full_json")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Setting backed up successfully!"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    else
        print_error "Error backing up setting (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
}

# Main logic - backup single file
backup_file "$FILE_PATH" "$DEVICE_ID"

