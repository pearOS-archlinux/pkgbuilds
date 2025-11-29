#!/bin/bash

# Complete script for authentication and application synchronization
# Usage example: ./login_and_sync.sh your@email.com yourpassword

# Configuration: API Base URL (can be overridden by PEARID_API_URL environment variable)
API_BASE_URL="${PEARID_API_URL:-https://account.pearos.xyz/api}"

# Load token storage helper
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/token_storage.sh" 2>/dev/null || {
    # Fallback if helper doesn't exist
    TOKEN_FILE="$HOME/.pearid_token"
    save_token() {
        echo "$1" > "$TOKEN_FILE"
        chmod 600 "$TOKEN_FILE"
    }
}

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

# Check arguments
if [ $# -lt 2 ]; then
    echo "Usage: $0 <email> <password>"
    exit 1
fi

EMAIL="$1"
PASSWORD="$2"

# Basic email validation
if [[ ! "$EMAIL" =~ ^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$ ]]; then
    print_error "Invalid email format: $EMAIL"
    exit 1
fi

print_info "Authenticating with $EMAIL..."

# Generate device ID
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEVICE_ID=""
if [ -f "$SCRIPT_DIR/device_id.sh" ]; then
    DEVICE_ID=$(bash "$SCRIPT_DIR/device_id.sh" 2>/dev/null)
fi

# Get device name
DEVICE_NAME=$(hostname)

# Build login JSON
if [ -n "$DEVICE_ID" ]; then
    LOGIN_JSON="{\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\",\"device_id\":\"$DEVICE_ID\",\"device_name\":\"$DEVICE_NAME\"}"
else
    LOGIN_JSON="{\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\",\"device_name\":\"$DEVICE_NAME\"}"
fi

# Login
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/login.php" \
    -H "Content-Type: application/json" \
    -d "$LOGIN_JSON")

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -eq 200 ]; then
    TOKEN=$(echo "$BODY" | jq -r '.token')
    
    if [ "$TOKEN" != "null" ] && [ -n "$TOKEN" ]; then
        save_token "$TOKEN"
        print_success "Authentication successful! Token saved securely."
        
        # Call synchronization script
        if [ -f "$(dirname "$0")/sync_apps.sh" ]; then
            print_info "Synchronizing applications..."
            bash "$(dirname "$0")/sync_apps.sh"
        else
            print_info "Run manually: bash sync_apps.sh"
        fi
    else
        print_error "Token not received in response"
        echo "Server response: $BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
else
    print_error "Authentication failed (HTTP $HTTP_CODE)"
    
    # Extract error message
    ERROR_MSG=$(echo "$BODY" | jq -r '.error' 2>/dev/null)
    
    if [ -n "$ERROR_MSG" ] && [ "$ERROR_MSG" != "null" ]; then
        print_error "Error: $ERROR_MSG"
        
        # Suggestions for common errors
        if [ "$HTTP_CODE" -eq 401 ]; then
            echo ""
            print_info "Suggestions:"
            echo "  - Check if email is correct: $EMAIL"
            echo "  - Check if password is correct"
            echo "  - Make sure the account exists in the system"
        elif [ "$HTTP_CODE" -eq 400 ]; then
            echo ""
            print_info "Check if you provided correct email and password"
        fi
    else
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    fi
    
    exit 1
fi
