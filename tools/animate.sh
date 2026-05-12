#!/bin/bash

# Animate 3x3 cube grid with all rotation combinations
# Generates 180 frames with rotation 0-179 degrees

FRAMES=180
OUTPUT_DIR="frames"
SCENE_FILE="scene_tmp.cfg"
VIDEO_NAME="cube_grid_rotations.mp4"

mkdir -p "$OUTPUT_DIR"

echo "Generating $FRAMES frames..."
START_TIME=$(date +%s)

for i in $(seq 0 $((FRAMES - 1))); do
    printf "Frame %3d/%d\r" $((i + 1)) $FRAMES

    # Generate scene file with current rotation
    cat > "$SCENE_FILE" << EOF
camera:
{
    resolution = { width = 1200; height = 1200; };
    position = { x = 0; y = -50; z = -80; };
    look_at = { x = 0; y = 0; z = 0; };
    up = { x = 0; y = -1; z = 0; };
    fieldOfView = 60.0;
};

renderer:
{
    antialiasing = {
        enabled = false;
        method = "ssaa";
        samples = 4;
    };

    lighting = {
        ambientColor = { r = 30; g = 30; b = 30; };
        ambientMultiplier = 0.2;
        diffuseMultiplier = 0.8;
    };

    background = {
        color = { r = 40; g = 40; b = 50; };
    };

    output = "$OUTPUT_DIR/frame_$(printf "%04d" $i).png";
};

materials:
{
    coloreddiffuse = (
        { name = "white_floor"; color = { r = 200; g = 200; b = 200; }; }
    );

    phong = (
        { name = "red"; color = { r = 255; g = 50; b = 50; }; shininess = 32.0; },
        { name = "orange"; color = { r = 255; g = 150; b = 50; }; shininess = 32.0; },
        { name = "yellow"; color = { r = 255; g = 255; b = 50; }; shininess = 32.0; },
        { name = "green"; color = { r = 50; g = 255; b = 50; }; shininess = 32.0; },
        { name = "cyan"; color = { r = 50; g = 200; b = 255; }; shininess = 32.0; },
        { name = "blue"; color = { r = 50; g = 100; b = 255; }; shininess = 32.0; },
        { name = "purple"; color = { r = 200; g = 50; b = 255; }; shininess = 32.0; },
        { name = "magenta"; color = { r = 255; g = 50; b = 200; }; shininess = 32.0; },
        { name = "white"; color = { r = 255; g = 255; b = 255; }; shininess = 32.0; }
    );
};

shapes:
{
    planes = (
        {
            position = { x = 0; y = 15; z = 0; };
            normal = { x = 0; y = -1; z = 0; };
            material = "white_floor";
        }
    );

    boxes = (
        // Row 1 (back) - rotate on X, Y, Z individually
        { position = { x = -20; y = 0; z = 20; }; width = 8; height = 8; depth = 8; material = "red"; rotation = { x = $i; y = 0; z = 0; }; },
        { position = { x = 0; y = 0; z = 20; }; width = 8; height = 8; depth = 8; material = "orange"; rotation = { x = 0; y = $i; z = 0; }; },
        { position = { x = 20; y = 0; z = 20; }; width = 8; height = 8; depth = 8; material = "yellow"; rotation = { x = 0; y = 0; z = $i; }; },

        // Row 2 (middle) - rotate on XY, YZ, XZ combinations
        { position = { x = -20; y = 0; z = 0; }; width = 8; height = 8; depth = 8; material = "green"; rotation = { x = $i; y = $i; z = 0; }; },
        { position = { x = 0; y = 0; z = 0; }; width = 8; height = 8; depth = 8; material = "cyan"; rotation = { x = 0; y = $i; z = $i; }; },
        { position = { x = 20; y = 0; z = 0; }; width = 8; height = 8; depth = 8; material = "blue"; rotation = { x = $i; y = 0; z = $i; }; },

        // Row 3 (front) - rotate on XYZ, reverse, different speeds
        { position = { x = -20; y = 0; z = -20; }; width = 8; height = 8; depth = 8; material = "purple"; rotation = { x = $i; y = $i; z = $i; }; },
        { position = { x = 0; y = 0; z = -20; }; width = 8; height = 8; depth = 8; material = "magenta"; rotation = { x = $((FRAMES - 1 - i)); y = $i; z = $((FRAMES - 1 - i)); }; },
        { position = { x = 20; y = 0; z = -20; }; width = 8; height = 8; depth = 8; material = "white"; rotation = { x = $((i * 2 % 360)); y = $((i * 3 % 360)); z = $((i * 4 % 360)); }; }
    );
};

lights:
{
    point = (
        {
            position = { x = 0; y = -40; z = -30; };
            color = { r = 255; g = 255; b = 255; };
            intensity = 2000;
        },
        {
            position = { x = 30; y = -20; z = 0; };
            color = { r = 255; g = 255; b = 255; };
            intensity = 800;
        }
    );
};
EOF

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
