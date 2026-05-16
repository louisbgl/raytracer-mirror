#!/bin/bash

FRAMES=600
FRAMERATE=30
OUTPUT_DIR="frames"
SCENE_FILE="scene_tmp.cfg"
VIDEO_NAME="menger_zoom.mp4"

mkdir -p "$OUTPUT_DIR"

echo "Generating $FRAMES frames..."
START_TIME=$(date +%s)

for i in $(seq 0 $((FRAMES - 1))); do
    printf "Frame %3d/%d\r" $((i + 1)) $FRAMES

    # Compute zoom (FOV decreases over time)
    # Start FOV=60, end FOV=1
    fov=$(echo "60 - ($i / ($FRAMES - 1)) * 59" | bc -l)

    # Generate scene file with current FOV
    cat > "$SCENE_FILE" << EOF
camera:
{
    resolution = { width = 2000; height = 2000; };
    position = { x = 3.0; y = 2.5; z = 3.5; };
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
        color = { r = 20; g = 20; b = 30; };
    };

    toneMapping = {
        enabled = false;
    };
};

materials:
{
    phong = (
        { name = "white"; color = { r = 220; g = 220; b = 220; }; shininess = 32.0; },
        { name = "gold"; color = { r = 255; g = 215; b = 0; }; shininess = 64.0; }
    );
};

shapes:
{
    menger_sponges = (
        { position = { x = 0.0; y = 0.0; z = 0.0; }; iterations = 5; material = "gold"; }
    );

    planes = (
        { position = { x = 0.0; y = -1.5; z = 0.0; }; normal = { x = 0.0; y = 1.0; z = 0.0; }; material = "white"; }
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
