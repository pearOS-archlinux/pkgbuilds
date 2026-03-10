#!/bin/bash

# Script to download logged user information
# Downloads avatar and displays name, surname
# Usage: ./get_user_info.sh [--phone|--email|--first-name|--last-name|--billing-address|--birthdate|--country|--avatar|--apps|--all]

# Configuration: API Base URL (can be overridden by PEARID_API_URL environment variable)
API_BASE_URL="${PEARID_API_URL:-https://account.pearos.xyz/api}"

# Flags for selective display
SHOW_PHONE=false
SHOW_EMAIL=false
SHOW_FIRST_NAME=false
SHOW_LAST_NAME=false
SHOW_BILLING_ADDRESS=false
SHOW_BIRTHDATE=false
SHOW_COUNTRY=false
SHOW_AVATAR=false
SHOW_APPS=false
SHOW_DEVICES=false
SHOW_THIS_DEVICE=false
SHOW_ALL=true  # Default: show all
SHOW_JSON=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --phone)
            SHOW_PHONE=true
            SHOW_ALL=false
            shift
            ;;
        --email)
            SHOW_EMAIL=true
            SHOW_ALL=false
            shift
            ;;
        --first-name)
            SHOW_FIRST_NAME=true
            SHOW_ALL=false
            shift
            ;;
        --last-name)
            SHOW_LAST_NAME=true
            SHOW_ALL=false
            shift
            ;;
        --billing-address)
            SHOW_BILLING_ADDRESS=true
            SHOW_ALL=false
            shift
            ;;
        --birthdate)
            SHOW_BIRTHDATE=true
            SHOW_ALL=false
            shift
            ;;
        --country)
            SHOW_COUNTRY=true
            SHOW_ALL=false
            shift
            ;;
        --avatar)
            SHOW_AVATAR=true
            SHOW_ALL=false
            shift
            ;;
        --apps)
            SHOW_APPS=true
            SHOW_ALL=false
            shift
            ;;
        --devices)
            SHOW_DEVICES=true
            SHOW_ALL=false
            shift
            ;;
        --this)
            SHOW_THIS_DEVICE=true
            SHOW_ALL=false
            shift
            ;;
        --all)
            SHOW_ALL=true
            shift
            ;;
        --json)
            SHOW_JSON=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --phone            Display only phone number"
            echo "  --email            Display only email"
            echo "  --first-name       Display only first name"
            echo "  --last-name        Display only last name"
            echo "  --billing-address  Display only billing address"
            echo "  --birthdate        Display only birthdate"
            echo "  --country          Display only country/region"
            echo "  --avatar           Download only avatar"
            echo "  --apps             Display only applications"
            echo "  --devices          Display only connected devices"
            echo "  --this             Display only current device (this device)"
            echo "  --all              Display all information (default)"
            echo "  --json             Output JSON (compatible with other options)"
            echo "  --help             Show this message"
            echo ""
            echo "Examples:"
            echo "  $0 --phone"
            echo "  $0 --email --phone"
            echo "  $0 --apps"
            echo "  $0 --all"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help to see available options"
            exit 1
            ;;
    esac
done

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
NC='\033[0m' # No Color

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

# Function to convert country code to country name
get_country_name() {
    local code="$1"
    case "$code" in
        US) echo "United States" ;;
        GB) echo "United Kingdom" ;;
        CA) echo "Canada" ;;
        AU) echo "Australia" ;;
        DE) echo "Germany" ;;
        FR) echo "France" ;;
        IT) echo "Italy" ;;
        ES) echo "Spain" ;;
        NL) echo "Netherlands" ;;
        BE) echo "Belgium" ;;
        CH) echo "Switzerland" ;;
        AT) echo "Austria" ;;
        SE) echo "Sweden" ;;
        NO) echo "Norway" ;;
        DK) echo "Denmark" ;;
        FI) echo "Finland" ;;
        PL) echo "Poland" ;;
        CZ) echo "Czech Republic" ;;
        IE) echo "Ireland" ;;
        PT) echo "Portugal" ;;
        GR) echo "Greece" ;;
        RO) echo "Romania" ;;
        HU) echo "Hungary" ;;
        BG) echo "Bulgaria" ;;
        HR) echo "Croatia" ;;
        SK) echo "Slovakia" ;;
        SI) echo "Slovenia" ;;
        EE) echo "Estonia" ;;
        LV) echo "Latvia" ;;
        LT) echo "Lithuania" ;;
        LU) echo "Luxembourg" ;;
        MT) echo "Malta" ;;
        CY) echo "Cyprus" ;;
        IS) echo "Iceland" ;;
        JP) echo "Japan" ;;
        CN) echo "China" ;;
        KR) echo "South Korea" ;;
        IN) echo "India" ;;
        BR) echo "Brazil" ;;
        MX) echo "Mexico" ;;
        AR) echo "Argentina" ;;
        CL) echo "Chile" ;;
        CO) echo "Colombia" ;;
        PE) echo "Peru" ;;
        ZA) echo "South Africa" ;;
        EG) echo "Egypt" ;;
        NG) echo "Nigeria" ;;
        KE) echo "Kenya" ;;
        GH) echo "Ghana" ;;
        AE) echo "United Arab Emirates" ;;
        SA) echo "Saudi Arabia" ;;
        IL) echo "Israel" ;;
        TR) echo "Turkey" ;;
        RU) echo "Russia" ;;
        UA) echo "Ukraine" ;;
        BY) echo "Belarus" ;;
        KZ) echo "Kazakhstan" ;;
        SG) echo "Singapore" ;;
        MY) echo "Malaysia" ;;
        TH) echo "Thailand" ;;
        VN) echo "Vietnam" ;;
        PH) echo "Philippines" ;;
        ID) echo "Indonesia" ;;
        NZ) echo "New Zealand" ;;
        FJ) echo "Fiji" ;;
        PK) echo "Pakistan" ;;
        BD) echo "Bangladesh" ;;
        LK) echo "Sri Lanka" ;;
        *) echo "$code" ;;
    esac
}

# Get token
TOKEN=$(get_token)

if [ -z "$TOKEN" ]; then
    print_error "Token not found."
    print_info "Please authenticate first:"
    echo ""
    echo "  ./login_and_sync.sh your@email.com yourpassword"
    echo ""
    echo "Or set environment variable:"
    echo "  export PEARID_TOKEN=\$(curl -X POST $API_BASE_URL/login.php \\"
    echo "    -H 'Content-Type: application/json' \\"
    echo "    -d '{\"email\":\"your@email.com\",\"password\":\"yourpassword\"}' \\"
    echo "    | jq -r '.token')"
    exit 1
fi

# Show message only if not in selective mode
if [ "$SHOW_ALL" = true ]; then
    print_info "Fetching user information..."
fi

# Get user data
RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/user.php" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json")

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -ne 200 ]; then
    if [ "$SHOW_ALL" = true ]; then
        print_error "Error fetching data (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    else
        # In selective mode, show only error without extra messages
        echo "$BODY" | jq -r '.error // "Error"' 2>/dev/null || echo "Error"
    fi
    
    if [ "$HTTP_CODE" -eq 401 ]; then
        if [ "$SHOW_ALL" = true ]; then
            print_info "Token expired. Please authenticate again."
        fi
        delete_token 2>/dev/null || true
    fi
    exit 1
fi

# Extract data
USER_DATA=$(echo "$BODY" | jq '.user')

if [ -z "$USER_DATA" ] || [ "$USER_DATA" = "null" ]; then
    if [ "$SHOW_ALL" = true ]; then
        print_error "User data not found"
    else
        # In selective mode, don't show anything or empty string
        exit 1
    fi
    exit 1
fi

# Extract data
FIRST_NAME=$(echo "$USER_DATA" | jq -r '.first_name')
LAST_NAME=$(echo "$USER_DATA" | jq -r '.last_name')
EMAIL=$(echo "$USER_DATA" | jq -r '.email')
PHONE_NUMBER=$(echo "$USER_DATA" | jq -r '.phone_number // empty')
BILLING_ADDRESS=$(echo "$USER_DATA" | jq -r '.billing_address // empty')
BIRTHDATE=$(echo "$USER_DATA" | jq -r '.birthdate // empty')
COUNTRY=$(echo "$USER_DATA" | jq -r '.country // empty')
AVATAR_URL=$(echo "$USER_DATA" | jq -r '.avatar // empty')
AVATAR_FILENAME=$(echo "$USER_DATA" | jq -r '.avatar_filename // empty')

# Display information selectively or all
if [ "$SHOW_ALL" = true ]; then
    echo ""
    echo "════════════════════════════════════════"
    echo "  User Information"
    echo "════════════════════════════════════════"
    echo ""
    
    print_data "First Name: $FIRST_NAME"
    print_data "Last Name: $LAST_NAME"
    print_data "Email: $EMAIL"
    
    if [ -n "$PHONE_NUMBER" ] && [ "$PHONE_NUMBER" != "null" ] && [ "$PHONE_NUMBER" != "" ]; then
        print_data "Phone: $PHONE_NUMBER"
    else
        print_data "Phone: (not set)"
    fi
    
    if [ -n "$BILLING_ADDRESS" ] && [ "$BILLING_ADDRESS" != "null" ] && [ "$BILLING_ADDRESS" != "" ]; then
        print_data "Billing Address: $BILLING_ADDRESS"
    else
        print_data "Billing Address: (not set)"
    fi
    
    if [ -n "$BIRTHDATE" ] && [ "$BIRTHDATE" != "null" ] && [ "$BIRTHDATE" != "" ]; then
        print_data "Birthdate: $BIRTHDATE"
    else
        print_data "Birthdate: (not set)"
    fi
    
    if [ -n "$COUNTRY" ] && [ "$COUNTRY" != "null" ] && [ "$COUNTRY" != "" ]; then
        # Convert country code to country name
        COUNTRY_NAME=$(get_country_name "$COUNTRY")
        print_data "Country/Region: $COUNTRY_NAME ($COUNTRY)"
    else
        print_data "Country/Region: (not set)"
    fi
else
    # Display only requested fields
    if [ "$SHOW_FIRST_NAME" = true ]; then
        echo "$FIRST_NAME"
    fi
    
    if [ "$SHOW_LAST_NAME" = true ]; then
        echo "$LAST_NAME"
    fi
    
    if [ "$SHOW_EMAIL" = true ]; then
        echo "$EMAIL"
    fi
    
    if [ "$SHOW_PHONE" = true ]; then
        if [ -n "$PHONE_NUMBER" ] && [ "$PHONE_NUMBER" != "null" ] && [ "$PHONE_NUMBER" != "" ]; then
            echo "$PHONE_NUMBER"
        else
            echo ""
        fi
    fi
    
    if [ "$SHOW_BILLING_ADDRESS" = true ]; then
        if [ -n "$BILLING_ADDRESS" ] && [ "$BILLING_ADDRESS" != "null" ] && [ "$BILLING_ADDRESS" != "" ]; then
            echo "$BILLING_ADDRESS"
        else
            echo ""
        fi
    fi
    
    if [ "$SHOW_BIRTHDATE" = true ]; then
        if [ -n "$BIRTHDATE" ] && [ "$BIRTHDATE" != "null" ] && [ "$BIRTHDATE" != "" ]; then
            echo "$BIRTHDATE"
        else
            echo ""
        fi
    fi
    
    if [ "$SHOW_COUNTRY" = true ]; then
        if [ -n "$COUNTRY" ] && [ "$COUNTRY" != "null" ] && [ "$COUNTRY" != "" ]; then
            # Convert country code to country name
            COUNTRY_NAME=$(get_country_name "$COUNTRY")
            echo "$COUNTRY_NAME"
        else
            echo ""
        fi
    fi
fi

# Download avatar if exists (only if --avatar or --all)
if ([ "$SHOW_AVATAR" = true ] || [ "$SHOW_ALL" = true ]) && [ -n "$AVATAR_URL" ] && [ "$AVATAR_URL" != "null" ] && [ "$AVATAR_URL" != "" ]; then
    echo ""
    print_info "Avatar found. Downloading..."
    
    # Create avatar directory if it doesn't exist
    AVATAR_DIR="$HOME/.pearid_avatars"
    mkdir -p "$AVATAR_DIR"
    
    # Always save as avatar.webp when using --avatar flag
    # If --all is used, keep the original filename for reference
    if [ "$SHOW_AVATAR" = true ] && [ "$SHOW_ALL" = false ]; then
        # --avatar only: save as avatar.webp
        AVATAR_FILENAME_LOCAL="avatar.webp"
    else
        # --all mode: use original filename or generate one
        if [ -n "$AVATAR_FILENAME" ] && [ "$AVATAR_FILENAME" != "null" ]; then
            AVATAR_FILENAME_LOCAL="$AVATAR_FILENAME"
        else
            AVATAR_FILENAME_LOCAL="avatar_${FIRST_NAME}_${LAST_NAME}_$(date +%Y%m%d_%H%M%S).webp"
        fi
    fi
    
    AVATAR_PATH="$AVATAR_DIR/$AVATAR_FILENAME_LOCAL"
    
    # Build full URL
    if [[ "$AVATAR_URL" == http* ]]; then
        AVATAR_FULL_URL="$AVATAR_URL"
    else
        # Assume it's a relative path
        AVATAR_FULL_URL="https://account.pearos.xyz$AVATAR_URL"
    fi
    
    # Download avatar
    if curl -s -f -o "$AVATAR_PATH" "$AVATAR_FULL_URL"; then
        print_success "Avatar downloaded: $AVATAR_PATH"
        
        # Display file information
        if command -v file &> /dev/null; then
            FILE_INFO=$(file "$AVATAR_PATH" 2>/dev/null)
            FILE_SIZE=$(stat -f%z "$AVATAR_PATH" 2>/dev/null || stat -c%s "$AVATAR_PATH" 2>/dev/null)
            FILE_SIZE_KB=$((FILE_SIZE / 1024))
            print_data "Size: ${FILE_SIZE_KB}KB"
            print_data "Type: $(echo "$FILE_INFO" | cut -d: -f2 | xargs)"
        fi
        
        # Open avatar if macOS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            read -p "Do you want to open the avatar? (y/N): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                open "$AVATAR_PATH"
            fi
        fi
    else
        print_error "Error downloading avatar"
        print_info "URL attempted: $AVATAR_FULL_URL"
    fi
else
    if [ "$SHOW_ALL" = true ]; then
        echo ""
        print_info "User has no avatar set"
    fi
fi

# Show separator only in full mode
if [ "$SHOW_ALL" = true ]; then
    echo ""
    echo "════════════════════════════════════════"
    echo ""
fi

# Get installed applications (only if --apps or --all)
if [ "$SHOW_APPS" = true ] || [ "$SHOW_ALL" = true ]; then
    if [ "$SHOW_ALL" = true ]; then
        echo ""
        print_info "Fetching installed applications..."
    fi

    APPS_RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/apps.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json")

    APPS_HTTP_CODE=$(echo "$APPS_RESPONSE" | tail -n1)
    APPS_BODY=$(echo "$APPS_RESPONSE" | sed '$d')

    if [ "$APPS_HTTP_CODE" -eq 200 ]; then
        APPS_DATA=$(echo "$APPS_BODY" | jq '.')
        UNIQUE_APPS=$(echo "$APPS_BODY" | jq -r '.unique_apps[]?')
        TOTAL_APPS=$(echo "$APPS_BODY" | jq -r '.unique_count // 0')
        
        if [ -n "$UNIQUE_APPS" ] && [ "$TOTAL_APPS" -gt 0 ]; then
            echo ""
            echo "════════════════════════════════════════"
            echo "  Installed Applications"
            echo "════════════════════════════════════════"
            echo ""
            print_data "Total unique applications: $TOTAL_APPS"
            echo ""
            
            # Display unique applications
            echo "$APPS_BODY" | jq -r '.unique_apps[]? | "  • \(.app_id) (on \(.device_count) devices) - last synced: \(.last_synced)"' | while IFS= read -r app_line; do
                if [ -n "$app_line" ]; then
                    print_data "$app_line"
                fi
            done
            
            # If there are apps on different devices, show details
            DEVICE_APPS=$(echo "$APPS_BODY" | jq -r '.apps[]? | select(.device_name != null) | .device_name' | sort -u)
            if [ -n "$DEVICE_APPS" ]; then
                echo ""
                print_info "Applications grouped by device:"
                for device in $(echo "$DEVICE_APPS"); do
                    DEVICE_COUNT=$(echo "$APPS_BODY" | jq -r ".apps[]? | select(.device_name == \"$device\") | .app_id" | wc -l | xargs)
                    if [ "$DEVICE_COUNT" -gt 0 ]; then
                        print_data "  $device: $DEVICE_COUNT applications"
                    fi
                done
            fi
            
            # If only --apps (without --all), show only simple list
            if [ "$SHOW_ALL" = false ] && [ "$SHOW_APPS" = true ]; then
                # Simple output: only appIDs, one per line
                echo "$APPS_BODY" | jq -r '.unique_apps[]?.app_id'
            else
                # Otherwise, show simple list after full details
                echo ""
                echo "════════════════════════════════════════"
                echo "  Simplified Applications List"
                echo "════════════════════════════════════════"
                echo ""
                echo "$APPS_BODY" | jq -r '.unique_apps[]?.app_id' | while IFS= read -r app_id; do
                    if [ -n "$app_id" ]; then
                        echo "$app_id"
                    fi
                done
            fi
        else
            if [ "$SHOW_ALL" = true ]; then
                echo ""
                print_info "No synchronized applications"
            fi
        fi
    else
        if [ "$SHOW_ALL" = true ]; then
            echo ""
            print_info "Could not fetch applications (HTTP $APPS_HTTP_CODE)"
            if [ "$APPS_HTTP_CODE" -ne 401 ]; then
                echo "$APPS_BODY" | jq '.' 2>/dev/null || echo "$APPS_BODY"
            fi
        fi
    fi
fi

# Get devices (only if --devices, --this, or --all)
if [ "$SHOW_DEVICES" = true ] || [ "$SHOW_THIS_DEVICE" = true ] || [ "$SHOW_ALL" = true ]; then
    if [ "$SHOW_ALL" = true ]; then
        echo ""
        print_info "Fetching connected devices..."
    fi

    # Detect current device name - try multiple variations
    CURRENT_HOSTNAME=$(hostname 2>/dev/null || echo "unknown")
    CURRENT_HOSTNAME_SHORT=$(hostname -s 2>/dev/null || echo "$CURRENT_HOSTNAME")
    CURRENT_HOSTNAME_FULL=$(hostname -f 2>/dev/null || echo "$CURRENT_HOSTNAME")
    
    # Build URL - always get all devices first, then filter locally for --this
    DEVICES_URL="$API_BASE_URL/devices.php"

    DEVICES_RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$DEVICES_URL" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json")

    DEVICES_HTTP_CODE=$(echo "$DEVICES_RESPONSE" | tail -n1)
    DEVICES_BODY=$(echo "$DEVICES_RESPONSE" | sed '$d')
    
    # Function to check if a device name matches current hostname
    is_current_device() {
        local device_name="$1"
        if [ -z "$device_name" ] || [ "$device_name" = "null" ] || [ "$device_name" = "" ]; then
            return 1
        fi
        
        # Try exact matches first
        if [ "$device_name" = "$CURRENT_HOSTNAME" ] || \
           [ "$device_name" = "$CURRENT_HOSTNAME_SHORT" ] || \
           [ "$device_name" = "$CURRENT_HOSTNAME_FULL" ]; then
            return 0
        fi
        
        # Try prefix/suffix matches (case-insensitive)
        device_lower=$(echo "$device_name" | tr '[:upper:]' '[:lower:]')
        hostname_lower=$(echo "$CURRENT_HOSTNAME" | tr '[:upper:]' '[:lower:]')
        hostname_short_lower=$(echo "$CURRENT_HOSTNAME_SHORT" | tr '[:upper:]' '[:lower:]')
        
        if [[ "$device_lower" == "$hostname_lower"* ]] || \
           [[ "$device_lower" == *"$hostname_lower" ]] || \
           [[ "$hostname_lower" == "$device_lower"* ]] || \
           [[ "$hostname_lower" == *"$device_lower" ]] || \
           [[ "$device_lower" == "$hostname_short_lower"* ]] || \
           [[ "$device_lower" == *"$hostname_short_lower" ]] || \
           [[ "$hostname_lower" == "$device_lower"* ]] || \
           [[ "$hostname_short_lower" == "$device_lower"* ]]; then
            return 0
        fi
        
        return 1
    }

    if [ "$DEVICES_HTTP_CODE" -eq 200 ]; then
        # Verify DEVICES_BODY is valid JSON
        if ! echo "$DEVICES_BODY" | jq empty 2>/dev/null; then
            if [ "$SHOW_ALL" = true ] || [ "$SHOW_THIS_DEVICE" = true ]; then
                print_error "Invalid JSON response from devices API"
            fi
            exit 1
        fi
        
        # For --this mode, filter devices to match current hostname
        if [ "$SHOW_THIS_DEVICE" = true ]; then
            # Filter devices to match current hostname (flexible matching)
            # First, get all device names
            ALL_DEVICE_NAMES=$(echo "$DEVICES_BODY" | jq -r '.devices[]?.device_name // empty' 2>/dev/null)
            
            # Try to find matching device using flexible matching
            MATCHED_DEVICE_NAME=""
            
            # Check each device name against hostname variations
            if [ -n "$ALL_DEVICE_NAMES" ]; then
                while IFS= read -r device_name; do
                    if is_current_device "$device_name"; then
                        MATCHED_DEVICE_NAME="$device_name"
                        break
                    fi
                done <<< "$ALL_DEVICE_NAMES"
            fi
            
            # If we found a match, filter the JSON
            if [ -n "$MATCHED_DEVICE_NAME" ]; then
                FILTERED_JSON=$(echo "$DEVICES_BODY" | jq --arg device_name "$MATCHED_DEVICE_NAME" \
                    '{
                        devices: [.devices[]? | select(.device_name == $device_name)],
                        total: 0
                    } | .total = (.devices | length)' 2>/dev/null)
                
                if [ -n "$FILTERED_JSON" ] && echo "$FILTERED_JSON" | jq empty 2>/dev/null; then
                    DEVICES_BODY="$FILTERED_JSON"
                else
                    DEVICES_BODY='{"devices":[],"total":0}'
                fi
            else
                # No match found
                DEVICES_BODY='{"devices":[],"total":0}'
            fi
        fi
        
        DEVICES_DATA=$(echo "$DEVICES_BODY" | jq '.' 2>/dev/null)
        DEVICES_LIST=$(echo "$DEVICES_BODY" | jq -r '.devices[]? // empty' 2>/dev/null)
        TOTAL_DEVICES=$(echo "$DEVICES_BODY" | jq -r '.total // 0' 2>/dev/null)
        
        if [ -n "$DEVICES_LIST" ] && [ "$TOTAL_DEVICES" -gt 0 ]; then
            if [ "$SHOW_ALL" = true ]; then
                echo ""
                echo "════════════════════════════════════════"
                echo "  Connected Devices"
                echo "════════════════════════════════════════"
                echo ""
                print_data "Total devices: $TOTAL_DEVICES"
                echo ""
            fi
            
            # Display devices
            if [ "$SHOW_ALL" = true ]; then
                # Full mode: show detailed device information
                echo "$DEVICES_BODY" | jq -r '.devices[]? | "\(.device_name) (\(.device_type)) - Last login: \(.last_login) - IP: \(.ip_address)"' | while IFS= read -r device_line; do
                    if [ -n "$device_line" ]; then
                        print_data "$device_line"
                    fi
                done
            elif [ "$SHOW_THIS_DEVICE" = true ]; then
                # --this mode: show only current device details
                DEVICE_OUTPUT=$(echo "$DEVICES_BODY" | jq -r '.devices[]? | "\(.device_name) (\(.device_type)) - Last login: \(.last_login) - IP: \(.ip_address)"' 2>/dev/null)
                if [ -n "$DEVICE_OUTPUT" ]; then
                    echo "$DEVICE_OUTPUT"
                fi
            else
                # Selective mode (--devices): show only device names, one per line, with * for current device
                echo "$DEVICES_BODY" | jq -r '.devices[]?.device_name' | while IFS= read -r device_name; do
                    if [ -n "$device_name" ] && [ "$device_name" != "null" ]; then
                        if is_current_device "$device_name"; then
                            echo "$device_name *"
                        else
                            echo "$device_name"
                        fi
                    fi
                done
            fi
        else
            if [ "$SHOW_ALL" = true ]; then
                echo ""
                print_info "No devices found"
            elif [ "$SHOW_THIS_DEVICE" = true ]; then
                # Current device not found - exit silently in selective mode
                # (This is expected if device hasn't logged in yet)
                exit 0
            fi
        fi
    else
        if [ "$SHOW_ALL" = true ]; then
            echo ""
            print_info "Could not fetch devices (HTTP $DEVICES_HTTP_CODE)"
            if [ "$DEVICES_HTTP_CODE" -ne 401 ]; then
                echo "$DEVICES_BODY" | jq '.' 2>/dev/null || echo "$DEVICES_BODY"
            fi
        fi
    fi
fi

# Optional: Export data as JSON
if [ "$SHOW_JSON" = true ]; then
    echo ""
    echo "════════════════════════════════════════"
    echo "  JSON Data"
    echo "════════════════════════════════════════"
    echo ""
    echo "User information:"
    echo "$USER_DATA" | jq '.'
    if [ "$SHOW_APPS" = true ] || [ "$SHOW_ALL" = true ]; then
        if [ "$APPS_HTTP_CODE" -eq 200 ]; then
            echo ""
            echo "Applications:"
            echo "$APPS_BODY" | jq '.'
        fi
    fi
    if [ "$SHOW_DEVICES" = true ] || [ "$SHOW_ALL" = true ]; then
        if [ "$DEVICES_HTTP_CODE" -eq 200 ]; then
            echo ""
            echo "Devices:"
            echo "$DEVICES_BODY" | jq '.'
        fi
    fi
fi


