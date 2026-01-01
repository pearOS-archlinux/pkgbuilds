#!/bin/bash

# Script to create a new PearID account via API and automatically log in
# Usage: ./create_account.sh --first-name "Alexandru" --last-name "Balan" --email "alex@example.com" --password "Parola1234+" [options]

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
    save_token() {
        echo "$1" > "$TOKEN_FILE"
        chmod 600 "$TOKEN_FILE"
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
    echo "Usage: $0 [options]"
    echo ""
    echo "Required options:"
    echo "  --first-name \"Name\"        First name (required)"
    echo "  --last-name \"Surname\"      Last name (required)"
    echo "  --email \"email@example.com\" Email address (required)"
    echo "  --password \"Password\"      Password (required)"
    echo ""
    echo "Optional options:"
    echo "  --birthdate \"YYYY-MM-DD\"   Birthdate (format: YYYY-MM-DD)"
    echo "  --phone \"+40712345678\"      Phone number"
    echo "  --billing-address \"Address\" Billing address"
    echo "  --help                      Show this message"
    echo ""
    echo "Examples:"
    echo "  $0 --first-name \"Alexandru\" --last-name \"Balan\" --email \"alex@example.com\" --password \"Parola1234+\""
    echo "  $0 --first-name \"John\" --last-name \"Doe\" --email \"john@example.com\" --password \"Pass123!\" --birthdate \"1990-01-15\" --phone \"+40712345678\""
}

# Parse arguments
FIRST_NAME=""
LAST_NAME=""
EMAIL=""
PASSWORD=""
BIRTHDATE=""
PHONE_NUMBER=""
BILLING_ADDRESS=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --first-name)
            FIRST_NAME="$2"
            shift 2
            ;;
        --last-name)
            LAST_NAME="$2"
            shift 2
            ;;
        --email)
            EMAIL="$2"
            shift 2
            ;;
        --password)
            PASSWORD="$2"
            shift 2
            ;;
        --birthdate)
            BIRTHDATE="$2"
            shift 2
            ;;
        --phone)
            PHONE_NUMBER="$2"
            shift 2
            ;;
        --billing-address)
            BILLING_ADDRESS="$2"
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

# Validate required fields
if [ -z "$FIRST_NAME" ]; then
    print_error "First name is required"
    show_usage
    exit 1
fi

if [ -z "$LAST_NAME" ]; then
    print_error "Last name is required"
    show_usage
    exit 1
fi

if [ -z "$EMAIL" ]; then
    print_error "Email is required"
    show_usage
    exit 1
fi

if [ -z "$PASSWORD" ]; then
    print_error "Password is required"
    show_usage
    exit 1
fi

# Validate email format
if [[ ! "$EMAIL" =~ ^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$ ]]; then
    print_error "Invalid email format: $EMAIL"
    exit 1
fi

# Validate birthdate format if provided
if [ -n "$BIRTHDATE" ]; then
    if [[ ! "$BIRTHDATE" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
        print_error "Invalid birthdate format. Use YYYY-MM-DD (e.g., 1990-01-15)"
        exit 1
    fi
fi

# Check if user is already logged in
TOKEN=$(get_token 2>/dev/null)
if [ -n "$TOKEN" ]; then
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
    
    # Try to extract email from token
    LOGGED_IN_EMAIL=$(extract_email_from_token "$TOKEN")
    
    if [ -n "$LOGGED_IN_EMAIL" ] && [ "$LOGGED_IN_EMAIL" != "null" ]; then
        print_error "You are already logged in as: $LOGGED_IN_EMAIL"
        echo ""
        print_info "Please log out first before creating a new account:"
        echo ""
        echo "  ./exit.sh exit"
        echo "  or"
        echo "  ./exit.sh exit-delete"
        echo ""
        exit 1
    else
        print_error "You are already logged in with an existing session."
        echo ""
        print_info "Please log out first before creating a new account:"
        echo ""
        echo "  ./exit.sh exit"
        echo "  or"
        echo "  ./exit.sh exit-delete"
        echo ""
        exit 1
    fi
fi

print_info "Creating PearID account for: $EMAIL"

# Build JSON data
REGISTER_DATA="{"
REGISTER_DATA="$REGISTER_DATA\"email\":\"$EMAIL\","
REGISTER_DATA="$REGISTER_DATA\"password\":\"$PASSWORD\","
REGISTER_DATA="$REGISTER_DATA\"first_name\":\"$FIRST_NAME\","
REGISTER_DATA="$REGISTER_DATA\"last_name\":\"$LAST_NAME\""

if [ -n "$BIRTHDATE" ]; then
    REGISTER_DATA="$REGISTER_DATA,\"birthdate\":\"$BIRTHDATE\""
fi

if [ -n "$PHONE_NUMBER" ]; then
    # Escape quotes for JSON
    PHONE_ESCAPED=$(echo "$PHONE_NUMBER" | sed 's/"/\\"/g')
    REGISTER_DATA="$REGISTER_DATA,\"phone_number\":\"$PHONE_ESCAPED\""
fi

if [ -n "$BILLING_ADDRESS" ]; then
    # Escape quotes for JSON
    BILLING_ESCAPED=$(echo "$BILLING_ADDRESS" | sed 's/"/\\"/g')
    REGISTER_DATA="$REGISTER_DATA,\"billing_address\":\"$BILLING_ESCAPED\""
fi

REGISTER_DATA="$REGISTER_DATA}"

# Send registration request
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/register.php" \
    -H "Content-Type: application/json" \
    -d "$REGISTER_DATA")

HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
BODY=$(echo "$RESPONSE" | sed '$d')

if [ "$HTTP_CODE" -eq 201 ]; then
    print_success "Account created successfully!"
    
    # Extract user information from response
    USER_ID=$(echo "$BODY" | jq -r '.user.id' 2>/dev/null)
    USER_EMAIL=$(echo "$BODY" | jq -r '.user.email' 2>/dev/null)
    USER_FIRST_NAME=$(echo "$BODY" | jq -r '.user.first_name' 2>/dev/null)
    USER_LAST_NAME=$(echo "$BODY" | jq -r '.user.last_name' 2>/dev/null)
    USER_COUNTRY=$(echo "$BODY" | jq -r '.user.country' 2>/dev/null)
    
    echo ""
    print_info "Account details:"
    if [ -n "$USER_ID" ] && [ "$USER_ID" != "null" ]; then
        print_data "User ID: $USER_ID"
    fi
    if [ -n "$USER_EMAIL" ] && [ "$USER_EMAIL" != "null" ]; then
        print_data "Email: $USER_EMAIL"
    fi
    if [ -n "$USER_FIRST_NAME" ] && [ "$USER_FIRST_NAME" != "null" ] && [ -n "$USER_LAST_NAME" ] && [ "$USER_LAST_NAME" != "null" ]; then
        print_data "Name: $USER_FIRST_NAME $USER_LAST_NAME"
    fi
    if [ -n "$USER_COUNTRY" ] && [ "$USER_COUNTRY" != "null" ]; then
        print_data "Country: $USER_COUNTRY"
    fi
    
    echo ""
    print_info "Automatically logging in..."
    
    # Generate device ID for login
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
    
    # Auto-login after account creation
    LOGIN_RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/login.php" \
        -H "Content-Type: application/json" \
        -d "$LOGIN_JSON")
    
    LOGIN_HTTP_CODE=$(echo "$LOGIN_RESPONSE" | tail -n1)
    LOGIN_BODY=$(echo "$LOGIN_RESPONSE" | sed '$d')
    
    if [ "$LOGIN_HTTP_CODE" -eq 200 ]; then
        TOKEN=$(echo "$LOGIN_BODY" | jq -r '.token' 2>/dev/null)
        
        if [ "$TOKEN" != "null" ] && [ -n "$TOKEN" ]; then
            save_token "$TOKEN"
            print_success "Authentication successful! Token saved securely."
            
            # Call synchronization script if it exists
            if [ -f "$SCRIPT_DIR/sync_apps.sh" ]; then
                print_info "Synchronizing applications..."
                bash "$SCRIPT_DIR/sync_apps.sh"
            else
                print_info "Run manually to sync applications:"
                echo "  ./sync_apps.sh"
            fi
        else
            print_error "Token not received in login response"
            print_info "You can log in manually with:"
            echo "  ./login_and_sync.sh $EMAIL [password]"
        fi
    else
        print_error "Auto-login failed (HTTP $LOGIN_HTTP_CODE)"
        LOGIN_ERROR_MSG=$(echo "$LOGIN_BODY" | jq -r '.error' 2>/dev/null)
        if [ -n "$LOGIN_ERROR_MSG" ] && [ "$LOGIN_ERROR_MSG" != "null" ]; then
            print_error "Error: $LOGIN_ERROR_MSG"
        fi
        print_info "You can log in manually with:"
        echo "  ./login_and_sync.sh $EMAIL [password]"
    fi
    
    exit 0
else
    print_error "Account creation failed (HTTP $HTTP_CODE)"
    
    # Extract error message
    ERROR_MSG=$(echo "$BODY" | jq -r '.error' 2>/dev/null)
    
    if [ -n "$ERROR_MSG" ] && [ "$ERROR_MSG" != "null" ]; then
        print_error "Error: $ERROR_MSG"
    else
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
    fi
    
    exit 1
fi

