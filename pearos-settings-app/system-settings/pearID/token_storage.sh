#!/bin/bash

# Helper for secure token management
# Supports: macOS Keychain, Linux Keyring, environment variables, file (with strict permissions)

TOKEN_KEY="pearid_api_token"
TOKEN_FILE="$HOME/.pearid_token"

# Functions for token storage/retrieval

# macOS - Use Keychain
save_token_keychain() {
    local token="$1"
    security add-generic-password -a "$USER" -s "$TOKEN_KEY" -w "$token" -U 2>/dev/null || \
    security add-generic-password -a "$USER" -s "$TOKEN_KEY" -w "$token" 2>/dev/null
}

get_token_keychain() {
    security find-generic-password -a "$USER" -s "$TOKEN_KEY" -w 2>/dev/null
}

delete_token_keychain() {
    security delete-generic-password -a "$USER" -s "$TOKEN_KEY" 2>/dev/null
}

# Linux - Use Secret Service (GNOME Keyring)
save_token_keyring() {
    local token="$1"
    if command -v secret-tool &> /dev/null; then
        echo -n "$token" | secret-tool store --label="PearID API Token" service pearid key token 2>/dev/null
    fi
}

get_token_keyring() {
    if command -v secret-tool &> /dev/null; then
        secret-tool lookup service pearid key token 2>/dev/null
    fi
}

delete_token_keyring() {
    if command -v secret-tool &> /dev/null; then
        secret-tool clear service pearid key token 2>/dev/null
    fi
}

# File with strict permissions (fallback)
save_token_file() {
    local token="$1"
    echo -n "$token" > "$TOKEN_FILE"
    chmod 600 "$TOKEN_FILE"
    # Set correct ownership
    [ -w "$TOKEN_FILE" ] && chown "$USER" "$TOKEN_FILE" 2>/dev/null
}

get_token_file() {
    if [ -f "$TOKEN_FILE" ] && [ -r "$TOKEN_FILE" ]; then
        # Check permissions (should be 600 or more restrictive)
        local perms=$(stat -f "%OLp" "$TOKEN_FILE" 2>/dev/null || stat -c "%a" "$TOKEN_FILE" 2>/dev/null)
        if [ "$perms" != "600" ] && [ "$perms" != "400" ]; then
            echo "Warning: Token file has insecure permissions ($perms). Should be 600." >&2
        fi
        cat "$TOKEN_FILE" 2>/dev/null
    fi
}

delete_token_file() {
    [ -f "$TOKEN_FILE" ] && rm -f "$TOKEN_FILE"
}

# Main functions (auto-detect which method to use)
save_token() {
    local token="$1"
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if command -v security &> /dev/null; then
            save_token_keychain "$token" && echo "Token saved to macOS Keychain" && return 0
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v secret-tool &> /dev/null; then
            save_token_keyring "$token" && echo "Token saved to Linux Keyring" && return 0
        fi
    fi
    
    # Fallback to file
    save_token_file "$token"
    echo "Token saved to file: $TOKEN_FILE (permissions: 600)"
}

get_token() {
    local token
    
    # Try keychain/keyring first
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if command -v security &> /dev/null; then
            token=$(get_token_keychain)
            [ -n "$token" ] && echo -n "$token" && return 0
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v secret-tool &> /dev/null; then
            token=$(get_token_keyring)
            [ -n "$token" ] && echo -n "$token" && return 0
        fi
    fi
    
    # Fallback to environment variable
    if [ -n "$PEARID_TOKEN" ]; then
        echo -n "$PEARID_TOKEN"
        return 0
    fi
    
    # Fallback to file
    token=$(get_token_file)
    [ -n "$token" ] && echo -n "$token" && return 0
    
    return 1
}

delete_token() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        delete_token_keychain
        delete_token_file
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        delete_token_keyring
        delete_token_file
    else
        delete_token_file
    fi
}

# Export for use in other scripts
export -f save_token get_token delete_token
