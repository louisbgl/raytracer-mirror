#!/bin/bash

FRAMES=90
FRAMERATE=30
OUTPUT_DIR="frames"
SCENE_FILE="scene_tmp.cfg"
VIDEO_NAME="julia_zoom_part2.mp4"

mkdir -p "$OUTPUT_DIR"

echo "Generating $FRAMES frames..."
START_TIME=$(date +%s)

for i in $(seq 0 $((FRAMES - 1))); do
    printf "Frame %3d/%d\r" $((i + 1)) $FRAMES

    # Compute zoom (FOV decreases over time)
    # Start FOV=20, end FOV=8
    fov=$(echo "20 - ($i / ($FRAMES - 1)) * 12" | bc -l)

    # Generate scene file with current FOV
    cat > "$SCENE_FILE" << EOF
camera:
{
    resolution = { width = 1000; height = 1000; };
    position = { x = 3.5; y = 2.5; z = 3.5; };
    look_at = { x = 0.0; y = 0.0; z = 0.0; };
    up = { x = 0.0; y = 1.0; z = 0.0; };
    fieldOfView = $fov;
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
        color = { r = 10; g = 10; b = 20; };
    };
};

materials:
{
    phong = (
        { name = "fractal"; color = { r = 255; g = 120; b = 60; }; shininess = 64.0; }
    );
};

shapes:
{
    julia_set_3ds = (
        { position = { x = 0.0; y = 0.0; z = 0.0; }; material = "fractal"; }
    );
};

lights:
{
    point = (
        { position = { x = 5.0; y = 5.0; z = 5.0; }; color = { r = 255; g = 255; b = 255; }; intensity = 80; },
        { position = { x = -3.0; y = 3.0; z = 2.0; }; color = { r = 200; g = 220; b = 255; }; intensity = 50; }
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
if ffmpeg -y -framerate $FRAMERATE -i "$OUTPUT_DIR/frame_%04d.png" -c:v libx264 -pix_fmt yuv420p "$VIDEO_NAME" > /dev/null 2>&1; then
    echo "Video saved: $VIDEO_NAME"
else
    echo "ERROR: ffmpeg failed"
    ffmpeg -y -framerate $FRAMERATE -i "$OUTPUT_DIR/frame_%04d.png" -c:v libx264 -pix_fmt yuv420p "$VIDEO_NAME" 2>&1
    exit 1
fi

# Cleanup
rm -f "$SCENE_FILE"
rm -rf "$OUTPUT_DIR"
echo "Cleanup complete."
