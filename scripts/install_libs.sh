#!/usr/bin/env bash
set -e

echo "==============================="
echo "======== INSTALL LIBS ========="
echo "==============================="


# List of libraries to ensure are installed
LIBRARIES=(
    "Adafruit GFX Library"
    "Adafruit SSD1306"
    "TinyGPSPlus"
    "ArduinoJson@7.4.2"
)

for LIB in "${LIBRARIES[@]}"; do
    LIB_NAME="${ENTRY%@*}"
    VERSION="${ENTRY#*@}"

    if arduino-cli lib list | grep -Fq "$LIB_NAME"; then
        echo "✅ Library '$LIB' is already installed."
    else
        echo "⬇️  Installing '$LIB'..."
        arduino-cli lib install "$LIB"
    fi
done
