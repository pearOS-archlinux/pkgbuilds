#!/bin/bash

# Script to check if user is logged in to PearID
# Returns: "true" if logged in with valid token, "false" if not

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

# Get token
TOKEN=$(get_token 2>/dev/null)

# Check if token exists
if [ -z "$TOKEN" ]; then
    echo "false"
    exit 0
fi

# Validate token by making a request to API
# Use timeout to detect connection issues
RESPONSE=$(curl -s -w "\n%{http_code}" --max-time 5 --connect-timeout 3 -X GET "$API_BASE_URL/user.php" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" 2>&1)

# Get curl exit code
CURL_EXIT_CODE=$?

# Extract HTTP code from response
HTTP_CODE=$(echo "$RESPONSE" | tail -n1)

# Check if curl failed (connection error, timeout, etc.)
if [ $CURL_EXIT_CODE -ne 0 ]; then
    # Check specific curl exit codes to distinguish between no internet and server down
    case $CURL_EXIT_CODE in
        6)
            # Exit code 6: Couldn't resolve host (DNS failure) - no internet
            echo "300"
            exit 0
            ;;
        7)
            # Exit code 7: Failed to connect to host - could be server down or no internet
            # For localhost, this usually means server is down
            # For remote hosts, could be either
            # We'll treat it as no internet (300) since we can't be 100% sure
            echo "300"
            exit 0
            ;;
        28)
            # Exit code 28: Operation timeout - could be server slow or no internet
            echo "300"
            exit 0
            ;;
        *)
            # Other curl errors - treat as connection issue (no internet)
            echo "300"
            exit 0
            ;;
    esac
fi

# If we got here, curl succeeded, so we have a valid HTTP response

# Check for server errors (500-599)
if [ "$HTTP_CODE" -ge 500 ] && [ "$HTTP_CODE" -lt 600 ]; then
    echo "500"
    exit 0
fi

# If HTTP code is 200, token is valid
if [ "$HTTP_CODE" -eq 200 ]; then
    echo "true"
    exit 0
else
    # Token invalid, expired, or unauthorized (401, 403, etc.)
    echo "false"
    exit 0
fi

