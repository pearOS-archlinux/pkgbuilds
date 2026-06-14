#!/bin/bash

# Script to check device lock status
# 
# Usage:
#   ./pearid-lock.sh --status    - Returns true/false if device_id exists in database
#   ./pearid-lock.sh              - Returns censored email address if device_id exists

# Configuration: API Base URL (can be overridden by PEARID_API_URL environment variable)
API_BASE_URL="${PEARID_API_URL:-https://account.pearos.xyz/api}"

# Load device_id generator
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Generate device_id locally
LOCAL_DEVICE_ID=""
if [ -f "$SCRIPT_DIR/device_id.sh" ]; then
    LOCAL_DEVICE_ID=$(bash "$SCRIPT_DIR/device_id.sh" 2>/dev/null)
fi

if [ -z "$LOCAL_DEVICE_ID" ]; then
    echo "Error: Could not generate device ID" >&2
    exit 1
fi

# Check if --status flag is provided
if [ "$1" = "--status" ]; then
    # Return true/false based on device existence
    RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/check_device.php?device_id=$LOCAL_DEVICE_ID" \
        -H "Content-Type: application/json" 2>/dev/null)
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        if command -v jq &> /dev/null; then
            EXISTS=$(echo "$BODY" | jq -r '.exists' 2>/dev/null)
            if [ "$EXISTS" = "true" ]; then
                echo "true"
                exit 0
            else
                echo "false"
                exit 0
            fi
        else
            # Fallback: check if email field exists and is not null
            if echo "$BODY" | grep -q '"email":null' 2>/dev/null; then
                echo "false"
                exit 0
            elif echo "$BODY" | grep -q '"email":' 2>/dev/null; then
                echo "true"
                exit 0
            else
                echo "false"
                exit 0
            fi
        fi
    else
        # Server error or connection issue
        echo "false"
        exit 1
    fi
else
    # Return censored email address
    RESPONSE=$(curl -s -w "\n%{http_code}" -X GET "$API_BASE_URL/check_device.php?device_id=$LOCAL_DEVICE_ID" \
        -H "Content-Type: application/json" 2>/dev/null)
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -eq 200 ]; then
        if command -v jq &> /dev/null; then
            EXISTS=$(echo "$BODY" | jq -r '.exists' 2>/dev/null)
            if [ "$EXISTS" = "true" ]; then
                EMAIL=$(echo "$BODY" | jq -r '.email' 2>/dev/null)
                if [ -n "$EMAIL" ] && [ "$EMAIL" != "null" ]; then
                    echo "$EMAIL"
                    exit 0
                else
                    echo ""
                    exit 0
                fi
            else
                echo ""
                exit 0
            fi
        else
            # Fallback: extract email using sed/grep
            EMAIL=$(echo "$BODY" | grep -o '"email":"[^"]*"' | sed 's/"email":"\([^"]*\)"/\1/' 2>/dev/null)
            if [ -n "$EMAIL" ] && [ "$EMAIL" != "null" ]; then
                echo "$EMAIL"
                exit 0
            else
                echo ""
                exit 0
            fi
        fi
    else
        # Server error or connection issue
        echo ""
        exit 1
    fi
fi

