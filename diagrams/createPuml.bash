#!/bin/bash

# Create the output directory if it doesn't exist
mkdir -p ../out

# Iterate through all .puml files in the puml directory
for file in *.puml; do
    # Get the filename without the path and extension
    filename=$(basename "$file" .puml)

    # Generate the SVG file
    plantuml -t svg "$file" -o "../out/$filename.svg"
done