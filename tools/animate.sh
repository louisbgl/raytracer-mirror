#!/bin/bash

FRAMES=360
OUTPUT_DIR="frames"
SCENE_FILE="scene_tmp.cfg"
VIDEO_NAME="mobius_spinning.mp4"

mkdir -p "$OUTPUT_DIR"

echo "Generating $FRAMES frames..."
START_TIME=$(date +%s)

for i in $(seq 0 $((FRAMES - 1))); do
    printf "Frame %3d/%d\r" $((i + 1)) $FRAMES

    # Generate scene file with current rotation
    cat > "$SCENE_FILE" << EOF
camera:
{
    resolution = { width = 800; height = 800; };
    position = { x = 8.0; y = 3.0; z = 8.0; };
    look_at = { x = 0.0; y = 0.0; z = 0.0; };
    up = { x = 0.0; y = 1.0; z = 0.0; };
    fieldOfView = 60.0;
};

renderer:
{
    output = "output.ppm";

    antialiasing = {
        enabled = false;
        method = "ssaa";
    };

    lighting = {
        ambientColor = { r = 40; g = 40; b = 60; };
        ambientMultiplier = 0.3;
        diffuseMultiplier = 0.7;
    };

    background = {
        color = { r = 20; g = 20; b = 30; };
    };
};

materials:
{
    phong = (
        { name = "strip"; color = { r = 180; g = 100; b = 220; }; shininess = 64.0; }
    );
};

shapes:
{
    mobius_strips = (
        { position = { x = 0.0; y = 0.0; z = 0.0; }; radius = 3.0; width = 1.5; thickness = 0.15; twists = 1; material = "strip"; rotation = { x = 30; y = $i; z = 0; }; }
    );
};

lights:
{
    point = (
        { position = { x = 5.0; y = 5.0; z = 5.0; }; color = { r = 255; g = 255; b = 255; }; intensity = 70; },
        { position = { x = -3.0; y = 3.0; z = 2.0; }; color = { r = 200; g = 220; b = 255; }; intensity = 40; }
    );
};
EOF

    # Replace output line with frame path
    sed -i "s|output = .*|output = \"$OUTPUT_DIR/frame_$(printf "%04d" $i).png\";|" "$SCENE_FILE"

    # Render frame
    ./raytracer "$SCENE_FILE" > /dev/null 2>&1
done

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
echo -e "\nDone rendering $FRAMES frames in ${ELAPSED}s ($(echo "scale=2; $ELAPSED / $FRAMES" | bc)s per frame)"

echo "Converting to video..."
if ffmpeg -y -framerate 30 -i "$OUTPUT_DIR/frame_%04d.png" -c:v libx264 -pix_fmt yuv420p "$VIDEO_NAME" > /dev/null 2>&1; then
    echo "Video saved: $VIDEO_NAME"
else
    echo "ERROR: ffmpeg failed"
    ffmpeg -y -framerate 30 -i "$OUTPUT_DIR/frame_%04d.png" -c:v libx264 -pix_fmt yuv420p "$VIDEO_NAME" 2>&1
    exit 1
fi

# Cleanup
rm -f "$SCENE_FILE"
rm -rf "$OUTPUT_DIR"
echo "Cleanup complete."
