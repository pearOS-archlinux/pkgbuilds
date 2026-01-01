#!/bin/bash

# Script to exit/logout from PearID
# Options:
#   exit          - Delete token from local system and logout
#   exit-delete   - Same as exit, plus delete ./user-data directory
#
# Requirements:
#   - A saved token must exist (script will exit if no token is found)
#   - Password is required (can be provided as argument or will be prompted)
#   - Email is automatically extracted from the saved token (no need to provide it)

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
    delete_token() {
        if [[ "$OSTYPE" == "darwin"* ]]; then
            security delete-generic-password -a "$USER" -s "pearid_api_token" 2>/dev/null
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            secret-tool clear service pearid key token 2>/dev/null
        fi
        [ -f "$TOKEN_FILE" ] && rm -f "$TOKEN_FILE"
    }
}

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
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

# Parse arguments
MODE=""
PASSWORD=""

if [ "$1" = "exit" ]; then
    MODE="exit"
    PASSWORD="$2"
elif [ "$1" = "exit-delete" ]; then
    MODE="exit-delete"
    PASSWORD="$2"
else
    echo "Usage: $0 [exit|exit-delete] [password]"
    echo ""
    echo "Options:"
    echo "  exit [password]          - Delete token from local system and logout"
    echo "  exit-delete [password]   - Same as exit, plus delete ./user-data directory"
    echo ""
    echo "Password can be provided as argument or will be prompted if not provided."
    exit 1
fi

# Get password if not provided as argument
if [ -z "$PASSWORD" ]; then
    read -sp "Enter password: " PASSWORD
    echo ""
    
    if [ -z "$PASSWORD" ]; then
        print_error "Password is required"
        exit 1
    fi
fi

# Try to get token from storage
TOKEN=$(get_token 2>/dev/null)

# Check if token exists - exit script requires a saved token
if [ -z "$TOKEN" ]; then
    print_error "No saved token found. You are not logged in."
    echo ""
    print_info "To log in, use:"
    echo "  ./login_and_sync.sh"
    exit 1
fi

# Function to extract email from JWT token
extract_email_from_token() {
    local token="$1"
    if [ -z "$token" ]; then
        return 1
    fi
    
    # JWT format: header.payload.signature
    # Extract payload (second part)
    local payload=$(echo "$token" | cut -d'.' -f2)
    
    if [ -z "$payload" ]; then
        return 1
    fi
    
    # Decode base64url (replace - with +, _ with /, add padding if needed)
    local decoded=$(echo "$payload" | tr -d '\n' | sed 's/-/+/g; s/_/\//g')
    
    # Add padding if needed
    local padding=$((4 - ${#decoded} % 4))
    if [ $padding -ne 4 ]; then
        decoded="${decoded}$(printf '%*s' $padding | tr ' ' '=')"
    fi
    
    # Decode and extract email using jq if available, otherwise use sed/grep
    if command -v jq &> /dev/null; then
        echo "$decoded" | base64 -d 2>/dev/null | jq -r '.email' 2>/dev/null
    else
        # Fallback: use sed/grep to extract email
        echo "$decoded" | base64 -d 2>/dev/null | grep -o '"email":"[^"]*"' | sed 's/"email":"\([^"]*\)"/\1/' 2>/dev/null
    fi
}

# Extract email from token (required)
EMAIL=$(extract_email_from_token "$TOKEN")

if [ -z "$EMAIL" ] || [ "$EMAIL" = "null" ]; then
    print_error "Could not extract email from token. Token may be invalid."
    print_info "Please log in again:"
    echo "  ./login_and_sync.sh"
    exit 1
fi

print_info "Email extracted from token: $EMAIL"

# Generate local device_id first (needed for login request)
LOCAL_DEVICE_ID=""
if [ -f "$SCRIPT_DIR/device_id.sh" ]; then
    LOCAL_DEVICE_ID=$(bash "$SCRIPT_DIR/device_id.sh" 2>/dev/null)
fi

# Authenticate with email-token (token is required for exit script)
# Include device_id to avoid "device already registered" error
print_info "Authenticating with saved token..."

# Build login request with device_id if available
if [ -n "$LOCAL_DEVICE_ID" ]; then
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/login.php" \
        -H "Content-Type: application/json" \
        -d "{\"email\":\"$EMAIL\",\"token\":\"$TOKEN\",\"password\":\"$PASSWORD\",\"device_id\":\"$LOCAL_DEVICE_ID\"}")
else
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/login.php" \
        -H "Content-Type: application/json" \
        -d "{\"email\":\"$EMAIL\",\"token\":\"$TOKEN\",\"password\":\"$PASSWORD\"}")
fi

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -eq 200 ]; then
    print_success "Authentication successful"
else
    ERROR_MSG=$(echo "$BODY" | jq -r '.error' 2>/dev/null)
    print_error "Authentication failed: ${ERROR_MSG:-Unknown error}"
    print_info "Token may be expired or invalid. Please log in again:"
    echo "  ./login_and_sync.sh"
    exit 1
fi

# Delete device from server if it matches (device_id was already generated above)
print_info "Checking device registration on server..."

# If we have a token and device_id, try to delete the device from server
if [ -n "$TOKEN" ] && [ -n "$LOCAL_DEVICE_ID" ]; then
    print_info "Fetching devices from server..."
    
    # Get list of devices from server
    DEVICES_RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/devices.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" 2>/dev/null)
    
    DEVICES_HTTP_CODE=$(echo "$DEVICES_RESPONSE" | tail -n1)
    DEVICES_BODY=$(echo "$DEVICES_RESPONSE" | sed '$d')
    
    if [ "$DEVICES_HTTP_CODE" -eq 200 ]; then
        # Check if any device matches our local device_id
        if command -v jq &> /dev/null; then
            # Use jq to find matching device_id
            MATCHING_DEVICE=$(echo "$DEVICES_BODY" | jq -r ".devices[] | select(.device_id == \"$LOCAL_DEVICE_ID\") | .device_id" 2>/dev/null)
            
            if [ -n "$MATCHING_DEVICE" ] && [ "$MATCHING_DEVICE" != "null" ]; then
                # Device found, delete it
                print_info "Device found on server, removing..."
                
                DELETE_RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE "$API_BASE_URL/devices.php" \
                    -H "Authorization: Bearer $TOKEN" \
                    -H "Content-Type: application/json" \
                    -d "{\"device_id\":\"$LOCAL_DEVICE_ID\"}" 2>/dev/null)
                
                DELETE_HTTP_CODE=$(echo "$DELETE_RESPONSE" | tail -n1)
                DELETE_BODY=$(echo "$DELETE_RESPONSE" | sed '$d')
                
                if [ "$DELETE_HTTP_CODE" -eq 200 ]; then
                    print_success "Device removed from server"
                else
                    ERROR_MSG=$(echo "$DELETE_BODY" | jq -r '.error' 2>/dev/null)
                    print_info "Could not remove device from server: ${ERROR_MSG:-Unknown error}"
                fi
            else
                print_info "Device not found on server (may have been already removed or never registered)"
            fi
        else
            # Fallback: try to delete directly (jq not available)
            print_info "Attempting to remove device from server..."
            
            DELETE_RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE "$API_BASE_URL/devices.php" \
                -H "Authorization: Bearer $TOKEN" \
                -H "Content-Type: application/json" \
                -d "{\"device_id\":\"$LOCAL_DEVICE_ID\"}" 2>/dev/null)
            
            DELETE_HTTP_CODE=$(echo "$DELETE_RESPONSE" | tail -n1)
            DELETE_BODY=$(echo "$DELETE_RESPONSE" | sed '$d')
            
            if [ "$DELETE_HTTP_CODE" -eq 200 ]; then
                print_success "Device removed from server"
            elif [ "$DELETE_HTTP_CODE" -eq 404 ]; then
                print_info "Device not found on server (may have been already removed)"
            else
                ERROR_MSG=$(echo "$DELETE_BODY" | jq -r '.error' 2>/dev/null || echo "$DELETE_BODY")
                print_info "Could not remove device from server: ${ERROR_MSG:-Unknown error}"
            fi
        fi
    else
        print_info "Could not fetch devices from server (token may be invalid)"
    fi
elif [ -z "$LOCAL_DEVICE_ID" ]; then
    print_info "Could not generate local device ID, skipping device removal"
fi

# Delete token from local system
print_info "Deleting token from local system..."
delete_token 2>/dev/null || true
print_success "Token deleted from local system"

# If exit-delete mode, also delete user-data directory
if [ "$MODE" = "exit-delete" ]; then
    USER_DATA_DIR="./user-data"
    if [ -d "$USER_DATA_DIR" ]; then
        print_info "Deleting user-data directory..."
        rm -rf "$USER_DATA_DIR"
        print_success "User-data directory deleted"
    else
        print_info "User-data directory does not exist, skipping"
    fi
fi

print_success "Logout complete!"



