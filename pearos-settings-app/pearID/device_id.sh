#!/bin/bash

# Script to generate unique device ID based on hardware information
# Works on Linux and macOS
# Usage: ./device_id.sh

generate_device_id() {
    local RAW=""
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux: Use DMI information
        RAW="$(cat /sys/class/dmi/id/product_uuid 2>/dev/null)"
        RAW="${RAW}$(cat /sys/class/dmi/id/board_serial 2>/dev/null)"
        RAW="${RAW}$(cat /sys/class/dmi/id/chassis_serial 2>/dev/null)"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS: Use IOPlatformUUID
        RAW="$(ioreg -rd1 -c IOPlatformExpertDevice 2>/dev/null | awk '/IOPlatformUUID/ {print $3}' | tr -d '\"')"
    fi
    
    # If we couldn't get hardware info, use fallback
    if [ -z "$RAW" ]; then
        # Fallback: Use hostname + MAC address
        local HOSTNAME=$(hostname)
        local MAC=""
        
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            MAC=$(cat /sys/class/net/*/address 2>/dev/null | head -1 | tr -d '\n')
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            MAC=$(ifconfig en0 2>/dev/null | grep ether | awk '{print $2}' | tr -d '\n')
        fi
        
        RAW="${HOSTNAME}${MAC}"
        
        # If still empty, use hostname + unique ID
        if [ -z "$RAW" ]; then
            RAW="${HOSTNAME}$(date +%s)"
        fi
    fi
    
    # Generate SHA256 hash
    echo -n "$RAW" | sha256sum | awk '{print $1}'
}

# Generate and output device ID
generate_device_id

