#!/bin/bash

# Script to update user information
# Can update: avatar, first name, last name, phone, billing address, birthdate, password, applications
# Usage: ./update_user_info.sh [--first-name "Name"] [--last-name "Surname"] [--birthdate "YYYY-MM-DD"] [--avatar "/path/to/image.jpg"] [--old-password "old" --new-password "new"] [--add-app "app_id"] [--remove-app "app_id"] [--rem-apps]

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

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# Usage function
show_usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --first-name \"Name\"        Update first name"
    echo "  --last-name \"Surname\"      Update last name"
    echo "  --phone \"+40712345678\"     Update phone number"
    echo "  --billing-address \"Address\" Update billing address"
    echo "  --birthdate \"YYYY-MM-DD\"   Update birthdate (format: YYYY-MM-DD)"
    echo "  --avatar \"/path/to/img\"   Update avatar"
    echo "  --old-password \"pass\"      Current password (required for password change)"
    echo "  --new-password \"pass\"      New password (required for password change)"
    echo "  --add-app \"app_id\"         Add an application (accepts any string)"
    echo "  --device-id \"device_id\"     Device ID (SHA256 hash) - optional, used with --add-app"
    echo "  --remove-app \"app_id\"      Remove an application"
    echo "  --rem-apps                  Remove all applications for the current user"
    echo "  --help                      Show this message"
    echo ""
    echo "Examples:"
    echo "  $0 --first-name \"John\" --last-name \"Doe\""
    echo "  $0 --phone \"+40712345678\""
    echo "  $0 --billing-address \"Example Street, No. 1, City\""
    echo "  $0 --birthdate \"1990-01-15\""
    echo "  $0 --avatar \"/Users/me/photo.jpg\""
    echo "  $0 --old-password \"oldpass\" --new-password \"newpass\""
    echo "  $0 --add-app \"com.example.app\""
    echo "  $0 --add-app \"com.example.app\" --device-id \"abc123...\""
    echo "  $0 --remove-app \"com.example.app\""
    echo "  $0 --rem-apps"
    echo "  $0 --first-name \"John\" --phone \"+40712345678\" --birthdate \"1990-01-15\" --avatar \"/Users/me/photo.jpg\""
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
FIRST_NAME=""
LAST_NAME=""
PHONE_NUMBER=""
BILLING_ADDRESS=""
BIRTHDATE=""
AVATAR_PATH=""
OLD_PASSWORD=""
NEW_PASSWORD=""
ADD_APP=""
DEVICE_ID=""
REMOVE_APP=""
REMOVE_ALL_APPS=false

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
        --phone)
            PHONE_NUMBER="$2"
            shift 2
            ;;
        --billing-address)
            BILLING_ADDRESS="$2"
            shift 2
            ;;
        --birthdate)
            BIRTHDATE="$2"
            shift 2
            ;;
        --avatar)
            AVATAR_PATH="$2"
            shift 2
            ;;
        --old-password)
            OLD_PASSWORD="$2"
            shift 2
            ;;
        --new-password)
            NEW_PASSWORD="$2"
            shift 2
            ;;
        --add-app)
            ADD_APP="$2"
            shift 2
            ;;
        --device-id)
            DEVICE_ID="$2"
            shift 2
            ;;
        --remove-app)
            REMOVE_APP="$2"
            shift 2
            ;;
        --rem-apps)
            REMOVE_ALL_APPS=true
            shift
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

# Check if password change is requested
if [ -n "$OLD_PASSWORD" ] || [ -n "$NEW_PASSWORD" ]; then
    if [ -z "$OLD_PASSWORD" ] || [ -z "$NEW_PASSWORD" ]; then
        print_error "Both --old-password and --new-password are required to change password"
        show_usage
        exit 1
    fi
fi

# Check if at least one option is specified
if [ -z "$FIRST_NAME" ] && [ -z "$LAST_NAME" ] && [ -z "$PHONE_NUMBER" ] && [ -z "$BILLING_ADDRESS" ] && [ -z "$BIRTHDATE" ] && [ -z "$AVATAR_PATH" ] && [ -z "$OLD_PASSWORD" ] && [ -z "$ADD_APP" ] && [ -z "$REMOVE_APP" ] && [ "$REMOVE_ALL_APPS" = false ]; then
    print_error "You must specify at least one update option"
    show_usage
    exit 1
fi

# Update user data (name, surname, phone, address, birthdate) if specified
if [ -n "$FIRST_NAME" ] || [ -n "$LAST_NAME" ] || [ -n "$PHONE_NUMBER" ] || [ -n "$BILLING_ADDRESS" ] || [ -n "$BIRTHDATE" ]; then
    print_info "Updating user data..."
    
    # Build JSON for update
    UPDATE_DATA="{"
    FIRST_FIELD=true
    
    if [ -n "$FIRST_NAME" ]; then
        if [ "$FIRST_FIELD" = false ]; then
            UPDATE_DATA="$UPDATE_DATA,"
        fi
        UPDATE_DATA="$UPDATE_DATA\"first_name\":\"$FIRST_NAME\""
        FIRST_FIELD=false
    fi
    
    if [ -n "$LAST_NAME" ]; then
        if [ "$FIRST_FIELD" = false ]; then
            UPDATE_DATA="$UPDATE_DATA,"
        fi
        UPDATE_DATA="$UPDATE_DATA\"last_name\":\"$LAST_NAME\""
        FIRST_FIELD=false
    fi
    
    if [ -n "$PHONE_NUMBER" ]; then
        if [ "$FIRST_FIELD" = false ]; then
            UPDATE_DATA="$UPDATE_DATA,"
        fi
        UPDATE_DATA="$UPDATE_DATA\"phone_number\":\"$PHONE_NUMBER\""
        FIRST_FIELD=false
    fi
    
    if [ -n "$BILLING_ADDRESS" ]; then
        if [ "$FIRST_FIELD" = false ]; then
            UPDATE_DATA="$UPDATE_DATA,"
        fi
        # Escape quotes for JSON
        BILLING_ESCAPED=$(echo "$BILLING_ADDRESS" | sed 's/"/\\"/g')
        UPDATE_DATA="$UPDATE_DATA\"billing_address\":\"$BILLING_ESCAPED\""
        FIRST_FIELD=false
    fi
    
    if [ -n "$BIRTHDATE" ]; then
        if [ "$FIRST_FIELD" = false ]; then
            UPDATE_DATA="$UPDATE_DATA,"
        fi
        # Validate date format (YYYY-MM-DD)
        if [[ ! "$BIRTHDATE" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
            print_error "Invalid birthdate format. Use YYYY-MM-DD (e.g., 1990-01-15)"
            exit 1
        fi
        UPDATE_DATA="$UPDATE_DATA\"birthdate\":\"$BIRTHDATE\""
        FIRST_FIELD=false
    fi
    
    UPDATE_DATA="$UPDATE_DATA}"
    
    # Update via API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X PUT "$API_BASE_URL/user.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$UPDATE_DATA")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Data updated successfully!"
        if [ -n "$FIRST_NAME" ]; then
            print_data "New first name: $FIRST_NAME"
        fi
        if [ -n "$LAST_NAME" ]; then
            print_data "New last name: $LAST_NAME"
        fi
        if [ -n "$PHONE_NUMBER" ]; then
            print_data "New phone: $PHONE_NUMBER"
        fi
        if [ -n "$BILLING_ADDRESS" ]; then
            print_data "New billing address: $BILLING_ADDRESS"
        fi
        if [ -n "$BIRTHDATE" ]; then
            print_data "New birthdate: $BIRTHDATE"
        fi
    else
        print_error "Error updating data (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

# Update password if specified
if [ -n "$OLD_PASSWORD" ] && [ -n "$NEW_PASSWORD" ]; then
    print_info "Changing password..."
    
    # Build JSON for password change
    PASSWORD_DATA="{\"old_password\":\"$OLD_PASSWORD\",\"new_password\":\"$NEW_PASSWORD\"}"
    
    # Update via API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X PUT "$API_BASE_URL/user.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$PASSWORD_DATA")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Password changed successfully!"
        print_info "All existing sessions have been revoked. Please log in again."
        # Check if response indicates re-login required
        if echo "$BODY" | jq -e '.requires_relogin' >/dev/null 2>&1; then
            print_info "Your token has been invalidated. Please run:"
            echo ""
            echo "  ./login_and_sync.sh your@email.com yournewpassword"
            echo ""
            # Remove token file to force re-login
            if [ -f "$TOKEN_FILE" ]; then
                rm -f "$TOKEN_FILE"
                print_info "Local token file has been removed."
            fi
        fi
    else
        print_error "Error changing password (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

# Update avatar if specified
if [ -n "$AVATAR_PATH" ]; then
    if [ ! -f "$AVATAR_PATH" ]; then
        print_error "File does not exist: $AVATAR_PATH"
        exit 1
    fi
    
    print_info "Updating avatar..."
    print_data "File: $AVATAR_PATH"
    
    # Check file type
    FILE_TYPE=$(file -b --mime-type "$AVATAR_PATH" 2>/dev/null)
    if [[ ! "$FILE_TYPE" =~ ^image/(jpeg|png|gif|webp)$ ]]; then
        print_error "File type not allowed: $FILE_TYPE"
        print_info "Use JPEG, PNG, GIF or WebP"
        exit 1
    fi
    
    # Check size (max 5MB)
    FILE_SIZE=$(stat -f%z "$AVATAR_PATH" 2>/dev/null || stat -c%s "$AVATAR_PATH" 2>/dev/null)
    MAX_SIZE=$((5 * 1024 * 1024))
    if [ "$FILE_SIZE" -gt "$MAX_SIZE" ]; then
        print_error "File is too large: $FILE_SIZE bytes (max 5MB)"
        exit 1
    fi
    
    # Upload avatar
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/avatar.php" \
        -H "Authorization: Bearer $TOKEN" \
        -F "avatar=@$AVATAR_PATH")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Avatar updated successfully!"
        AVATAR_URL=$(echo "$BODY" | jq -r '.avatar')
        if [ -n "$AVATAR_URL" ] && [ "$AVATAR_URL" != "null" ]; then
            print_data "Avatar URL: $AVATAR_URL"
        fi
    else
        print_error "Error updating avatar (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

# Add application if specified
if [ -n "$ADD_APP" ]; then
    print_info "Adding application: $ADD_APP"
    
    # Get device name (hostname)
    DEVICE_NAME=$(hostname)
    
    # Escape quotes for JSON
    APP_ID_ESCAPED=$(echo "$ADD_APP" | sed 's/"/\\"/g')
    DEVICE_NAME_ESCAPED=$(echo "$DEVICE_NAME" | sed 's/"/\\"/g')
    
    # Build JSON for adding app (with allow_any_string flag)
    if [ -n "$DEVICE_ID" ]; then
        # Validate device_id format (SHA256 hash: 64 hex characters)
        if [[ ! "$DEVICE_ID" =~ ^[a-f0-9]{64}$ ]]; then
            print_error "Invalid device_id format. Must be a 64-character hexadecimal string (SHA256 hash)"
            exit 1
        fi
        DEVICE_ID_ESCAPED=$(echo "$DEVICE_ID" | sed 's/"/\\"/g')
        APP_DATA="{\"app_id\":\"$APP_ID_ESCAPED\",\"device_name\":\"$DEVICE_NAME_ESCAPED\",\"device_id\":\"$DEVICE_ID_ESCAPED\",\"allow_any_string\":true}"
        print_info "Linking to device_id: $DEVICE_ID"
    else
        APP_DATA="{\"app_id\":\"$APP_ID_ESCAPED\",\"device_name\":\"$DEVICE_NAME_ESCAPED\",\"allow_any_string\":true}"
    fi
    
    # Add via API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$API_BASE_URL/apps.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$APP_DATA")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Application added successfully!"
        print_data "App ID: $ADD_APP"
        ADDED_COUNT=$(echo "$BODY" | jq -r '.added' 2>/dev/null)
        if [ -n "$ADDED_COUNT" ] && [ "$ADDED_COUNT" != "null" ]; then
            print_data "Added: $ADDED_COUNT"
        fi
    else
        print_error "Error adding application (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

# Remove application if specified
if [ -n "$REMOVE_APP" ]; then
    print_info "Removing application: $REMOVE_APP"
    
    # Escape quotes for JSON
    APP_ID_ESCAPED=$(echo "$REMOVE_APP" | sed 's/"/\\"/g')
    
    # Build JSON for removing app
    APP_DATA="{\"app_id\":\"$APP_ID_ESCAPED\"}"
    
    # Remove via API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE "$API_BASE_URL/apps.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$APP_DATA")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "Application removed successfully!"
        DELETED_COUNT=$(echo "$BODY" | jq -r '.deleted' 2>/dev/null)
        if [ -n "$DELETED_COUNT" ] && [ "$DELETED_COUNT" != "null" ] && [ "$DELETED_COUNT" -gt 0 ]; then
            print_data "Removed: $DELETED_COUNT instance(s)"
        else
            print_info "Application not found in database"
        fi
    else
        print_error "Error removing application (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

# Remove all applications if specified
if [ "$REMOVE_ALL_APPS" = true ]; then
    print_info "Removing all applications for current user..."
    
    # Confirm deletion
    echo ""
    print_warning "This will delete ALL applications for your account. This action cannot be undone."
    read -p "Are you sure you want to continue? (yes/no): " CONFIRM
    
    if [ "$CONFIRM" != "yes" ]; then
        print_info "Operation cancelled."
        exit 0
    fi
    
    # Build JSON for removing all apps
    APP_DATA="{\"delete_all\":true}"
    
    # Remove via API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE "$API_BASE_URL/apps.php" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$APP_DATA")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        print_success "All applications removed successfully!"
        DELETED_COUNT=$(echo "$BODY" | jq -r '.deleted' 2>/dev/null)
        if [ -n "$DELETED_COUNT" ] && [ "$DELETED_COUNT" != "null" ]; then
            print_data "Removed: $DELETED_COUNT application(s)"
        fi
    else
        print_error "Error removing all applications (HTTP $HTTP_CODE)"
        echo "$BODY" | jq '.' 2>/dev/null || echo "$BODY"
        exit 1
    fi
fi

echo ""
print_success "Update complete!"

# Display updated information
echo ""
print_info "Updated information:"
./get_user_info.sh --json 2>/dev/null || {
    print_info "Run './get_user_info.sh' to see updated information"
}
