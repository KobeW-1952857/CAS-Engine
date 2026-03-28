#!/bin/bash

# Ensure an input file was provided
if [ -z "$1" ]; then
    echo "Usage: ./generate_icon.sh <path_to_1024x1024_image.png>"
    exit 1
fi

INPUT_FILE=$1
ICONSET_DIR="CASEngine.iconset"

echo "Creating $ICONSET_DIR..."
mkdir -p "$ICONSET_DIR"

# Generate all the required resolutions using 'sips'
echo "Resizing images..."
sips -z 16 16     "$INPUT_FILE" --out "${ICONSET_DIR}/icon_16x16.png" > /dev/null
sips -z 32 32     "$INPUT_FILE" --out "${ICONSET_DIR}/icon_16x16@2x.png" > /dev/null
sips -z 32 32     "$INPUT_FILE" --out "${ICONSET_DIR}/icon_32x32.png" > /dev/null
sips -z 64 64     "$INPUT_FILE" --out "${ICONSET_DIR}/icon_32x32@2x.png" > /dev/null
sips -z 128 128   "$INPUT_FILE" --out "${ICONSET_DIR}/icon_128x128.png" > /dev/null
sips -z 256 256   "$INPUT_FILE" --out "${ICONSET_DIR}/icon_128x128@2x.png" > /dev/null
sips -z 256 256   "$INPUT_FILE" --out "${ICONSET_DIR}/icon_256x256.png" > /dev/null
sips -z 512 512   "$INPUT_FILE" --out "${ICONSET_DIR}/icon_256x256@2x.png" > /dev/null
sips -z 512 512   "$INPUT_FILE" --out "${ICONSET_DIR}/icon_512x512.png" > /dev/null
sips -z 1024 1024 "$INPUT_FILE" --out "${ICONSET_DIR}/icon_512x512@2x.png" > /dev/null

# Compile the .iconset folder into a .icns file
echo "Compiling to CASEngine.icns..."
iconutil -c icns "$ICONSET_DIR"

# Optional: Clean up the intermediate .iconset folder if you only want the .icns
rm -R "$ICONSET_DIR"

echo "Done! CASEngine.icns is ready."