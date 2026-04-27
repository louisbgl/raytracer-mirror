#!/usr/bin/env bash

set -euo pipefail

# Convert all common image formats in this directory to .ppm.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if command -v magick >/dev/null 2>&1; then
	IM_CMD=(magick)
elif command -v convert >/dev/null 2>&1; then
	IM_CMD=(convert)
else
	echo "Warning: ImageMagick not found (magick or convert command not found). Skipping texture conversion."
	exit 0
fi

shopt -s nullglob nocaseglob

converted=0
failed=0

for input in *.jpg *.jpeg *.png *.bmp *.gif *.tif *.tiff *.webp; do
	[ -f "$input" ] || continue

	output="${input%.*}.ppm"
	if "${IM_CMD[@]}" "$input" "$output"; then
		echo "Converted: $input -> $output"
		converted=$((converted + 1))
	else
		echo "Failed: $input"
		failed=$((failed + 1))
	fi
done

if [ "$converted" -eq 0 ] && [ "$failed" -eq 0 ]; then
	echo "No matching image files found to convert."
else
	echo "Done. Converted: $converted, Failed: $failed"
fi
