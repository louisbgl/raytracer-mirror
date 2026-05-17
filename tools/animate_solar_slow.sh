#!/bin/bash

FRAMES=360
FRAMERATE=30
OUTPUT_DIR="frames"
SCENE_FILE="solar_system.txt"
VIDEO_NAME="solar_system.mp4"

mkdir -p "$OUTPUT_DIR"

echo "Generating $FRAMES frames with naturally staggered starting orbits..."
START_TIME=$(date +%s)

sun_x=-22.0
sun_z=0.0
PI=$(echo "4*a(1)" | bc -l)

for i in $(seq 0 $((FRAMES - 1))); do
    printf "Progress: [%3d/%d frames]\r" $((i + 1)) $FRAMES

    # --- Axial Spin ---
    base_spin=$(echo "360 * ($i / $FRAMES)" | bc -l)
    r_sun=$(echo "$base_spin * 0.05" | bc -l)
    r_mercury=$(echo "$base_spin * 0.2" | bc -l)
    r_venus=$(echo "$base_spin * -0.05" | bc -l)
    r_earth=$(echo "$base_spin * 0.3" | bc -l)
    r_moon=$(echo "$base_spin * 0.3" | bc -l)
    r_mars=$(echo "$base_spin * 0.25" | bc -l)
    r_jupiter=$(echo "$base_spin * 0.5" | bc -l)
    r_saturn=$(echo "$base_spin * 0.45" | bc -l)
    r_uranus=$(echo "$base_spin * -0.3" | bc -l)
    r_neptune=$(echo "$base_spin * 0.3" | bc -l)
    r_pluto=$(echo "$base_spin * 0.1" | bc -l)

    # --- Slow Orbital Tracking ---
    base_orbit=$(echo "0.75 * 2 * $PI * ($i / $FRAMES)" | bc -l)

    # Updated math function accepts: Distance, Speed, and Initial Angle Offset (Degrees)
    calc_pos() {
        local dist=$1
        local speed=$2
        local start_deg=$3
        
        # Total angle = (time * speed) + starting offset converted to radians
        local angle=$(echo "$base_orbit * $speed + ($start_deg * $PI / 180)" | bc -l)
        current_x=$(echo "$sun_x + ($dist * s($angle))" | bc -l)
        current_z=$(echo "$sun_z + ($dist * c($angle))" | bc -l)
    }

    # Planets are now mapped with custom scattered starting positions (0 to 360 degrees)
    calc_pos 5.5  1.50   45;  x_mercury=$current_x; z_mercury=$current_z  
    calc_pos 7.5  0.85  120;  x_venus=$current_x;   z_venus=$current_z   
    calc_pos 10.0 0.50  200;  x_earth=$current_x;   z_earth=$current_z    
    
    # Moon remains dynamically bound close to Earth's moving position
    moon_angle=$(echo "$base_orbit * 3.0 + (15 * $PI / 180)" | bc -l)
    x_moon=$(echo "$x_earth + (0.9 * s($moon_angle))" | bc -l)
    z_moon=$(echo "$z_earth + (0.9 * c($moon_angle))" | bc -l)
    
    calc_pos 12.5 0.35  315;  x_mars=$current_x;    z_mars=$current_z
    calc_pos 15.5 0.18   90;  x_jupiter=$current_x; z_jupiter=$current_z 
    calc_pos 19.5 0.12  270;  x_saturn=$current_x;  z_saturn=$current_z  
    calc_pos 23.5 0.08  160;  x_uranus=$current_x;  z_uranus=$current_z  
    calc_pos 26.5 0.05   30;  x_neptune=$current_x; z_neptune=$current_z 
    calc_pos 29.0 0.03  230;  x_pluto=$current_x;   z_pluto=$current_z

    cat > "$SCENE_FILE" << EOF
camera:
{
    resolution = { width = 3000; height = 3000; };
    position = { x = -22.0; y = 32.0; z = 32.0; }; 
    look_at = { x = -22.0; y = 0.0; z = 0.0; };
    up = { x = 0.0; y = 1.0; z = 0.0; };
    fieldOfView = 80.0;
};

materials:
{
    image_texture = (
        { name = "sun";     path = "textures/sun.jpg"; },
        { name = "mercury"; path = "textures/mercury.jpg"; },
        { name = "venus";   path = "textures/venus.jpg"; },
        { name = "earth";   path = "textures/earth.jpg"; },
        { name = "moon";    path = "textures/moon.jpg"; },
        { name = "mars";    path = "textures/mars.jpg"; },
        { name = "jupiter"; path = "textures/jupiter.jpg"; },
        { name = "saturn";  path = "textures/saturn.jpg"; },
        { name = "uranus";  path = "textures/uranus.jpg"; },
        { name = "neptune"; path = "textures/neptune.jpg"; },
        { name = "pluto";   path = "textures/pluto.jpg"; }
    );

    perlinnoise = (
        { name = "space"; color = { r = 30; g = 30; b = 30; }; scale = 10.0; }
    );
};

shapes:
{
    planes = (
        { position = { x = 0.0; y = -4.0; z = 0.0; }; normal = { x = 0.0; y = 1.0; z = 0.0; }; material = "space"; }
    );

    spheres = (
        { position = { x = $sun_x; y = 0.0; z = $sun_z; }; radius = 3.0;  material = "sun";     rotation = { x = 0.0; y = $r_sun; z = 0.0; }; },
        { position = { x = $x_mercury; y = 0.0; z = $z_mercury; }; radius = 0.25; material = "mercury"; rotation = { x = 0.0; y = $r_mercury; z = 0.0; }; },
        { position = { x = $x_venus;   y = 0.0; z = $z_venus;   }; radius = 0.45; material = "venus";   rotation = { x = 0.0; y = $r_venus; z = 0.0; }; },
        { position = { x = $x_earth;   y = 0.0; z = $z_earth;   }; radius = 0.5;  material = "earth";   rotation = { x = 0.0; y = $r_earth; z = 0.0; }; },
        { position = { x = $x_moon;    y = 0.5; z = $z_moon;    }; radius = 0.15; material = "moon";    rotation = { x = 0.0; y = $r_moon; z = 0.0; }; },
        { position = { x = $x_mars;    y = 0.0; z = $z_mars;    }; radius = 0.35; material = "mars";    rotation = { x = 0.0; y = $r_mars; z = 0.0; }; },
        { position = { x = $x_jupiter; y = 0.0; z = $z_jupiter; }; radius = 1.5;  material = "jupiter"; rotation = { x = 0.0; y = $r_jupiter; z = 0.0; }; },
        { position = { x = $x_saturn;  y = 0.0; z = $z_saturn;  }; radius = 1.2;  material = "saturn";  rotation = { x = 0.0; y = $r_saturn; z = 0.0; }; },
        { position = { x = $x_uranus;  y = 0.0; z = $z_uranus;  }; radius = 0.7;  material = "uranus";  rotation = { x = 0.0; y = $r_uranus; z = 0.0; }; },
        { position = { x = $x_neptune; y = 0.0; z = $z_neptune; }; radius = 0.65; material = "neptune"; rotation = { x = 0.0; y = $r_neptune; z = 0.0; }; },
        { position = { x = $x_pluto;   y = 0.0; z = $z_pluto;   }; radius = 0.2;  material = "pluto";   rotation = { x = 0.0; y = $r_pluto; z = 0.0; }; }
    );
};

lights:
{
    ambient = 0.15;
    diffuse = 0.85;

    directional = (
        { direction = { x = 0.2; y = -0.6; z = -1.0; }; color = { r = 255; g = 245; b = 220; }; intensity = 1.0; }
    );

    point = (
        { position = { x = -22.0; y = 0.0; z = 0.0; }; color = { r = 255; g = 250; b = 240;}; intensity = 150; }
    );
};
EOF

    sed -i "1s|^|renderer = { output = \"$OUTPUT_DIR/frame_$(printf "%04d" $i).png\"; };\n|" "$SCENE_FILE"
    ./raytracer "$SCENE_FILE" > /dev/null 2>&1
done

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
echo -e "\nDone rendering $FRAMES frames in ${ELAPSED}s ($(echo "scale=2; $ELAPSED / $FRAMES" | bc)s per frame)"

echo "Converting to video..."
if ffmpeg -y -framerate $FRAMERATE -i "$OUTPUT_DIR/frame_%04d.png" -c:v libx264 -pix_fmt yuv420p "$VIDEO_NAME" > /dev/null 2>&1; then
    echo "Video saved successfully: $VIDEO_NAME"
else
    echo "ERROR: ffmpeg execution failed."
    exit 1
fi

rm -f "$SCENE_FILE"
rm -rf "$OUTPUT_DIR"
echo "Cleanup complete."
