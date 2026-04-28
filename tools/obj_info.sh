#!/bin/bash
# usage: obj_info.sh <file.obj>
awk '
    /^v / {
        if (NR == 1 || $2 < xmin) xmin = $2;  if ($2 > xmax) xmax = $2
        if (NR == 1 || $3 < ymin) ymin = $3;  if ($3 > ymax) ymax = $3
        if (NR == 1 || $4 < zmin) zmin = $4;  if ($4 > zmax) zmax = $4
        v++
    }
    /^f /  { f++; tri += NF - 3 }
    END {
        printf "vertices: %d  triangles: %d\n", v, tri
        printf "x: [%.4f, %.4f]  size: %.4f\n", xmin, xmax, xmax - xmin
        printf "y: [%.4f, %.4f]  size: %.4f\n", ymin, ymax, ymax - ymin
        printf "z: [%.4f, %.4f]  size: %.4f\n", zmin, zmax, zmax - zmin
    }
' "$1"
