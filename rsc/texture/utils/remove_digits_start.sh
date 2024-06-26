#!/bin/bash

source ../sh/color.sh

# Check if argument is provided
if [ -z "$1" ]; then
    display_color_msg ${YELLOW} "You must provide the directory to search for files."
    exit 1
fi

# Check if directory exists
if [ ! -d "$1" ]; then
    display_color_msg ${RED} "Directory $1 does not exist."
    exit 1
fi

dir="$1"

# Function to rename files
rename_files() {
    for file in "$dir"/*; do
        if [[ -f "$file" ]]; then
            filename=$(basename "$file")
            if [[ "$filename" =~ ^[0-9]+_(.*) ]]; then
                new_name="${BASH_REMATCH[1]}"
                mv "$file" "$dir/$new_name"
                display_color_msg ${GREEN} "Renamed: $filename to $new_name"
            fi
        fi
    done
}

# Rename files in the directory
rename_files
