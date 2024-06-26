#!/bin/bash

source ../sh/color.sh


# This script generates a dynamic enum from all BMP files in the given directory and stores it in the specified output file.
# The enum is created in reverse order of the BMP files to be consistent with the order in which they are loaded.
# The enum names are derived from the BMP filenames by converting spaces and hyphens to underscores and converting to uppercase.
# The output file contains the enum definition and a macro for the maximum enum value.
# After this we can create atlas with build_atlas.sh script on the same directory.

# How to use it: ./build_dymanic_enum.sh <directory> <output_file>

# Check if argument is provided
if [ -z "$1" ]; then
	display_color_msg ${YELLOW} "You must provide the directory to search for bmp files."
	exit 1
fi

# Check if directory exists
if [ ! -d "$1" ]; then
    display_color_msg ${RED} "Directory $1 does not exist."
    exit 1
fi

# Check if output file is provided
if [ -z "$2" ]; then
	display_color_msg ${YELLOW} "You must provide the output file."
	exit 1
fi

dir="$1"
output_file="$2"
save_last_enum=""

# Get all bmp files and store in an array
files=($(ls -1 "$dir" | grep '\.bmp$'))

# Transform filename to enum
filename_to_enum() {
    local filename=$1
    local base_name=${filename%.bmp}
    base_name=${base_name// /_}
    base_name=${base_name//-/_}
    echo "NEWAT_${base_name^^}"
}


# Create enum
create_enum_file() {
	echo "#ifndef HEADER_BLOCK_TYPE_H" > $output_file
	echo "#define HEADER_BLOCK_TYPE_H" >> $output_file
	echo "enum newAtlasID {" >> $output_file
	# Loop through the files array in reverse order
	for (( idx=${#files[@]}-1 ; idx>=0 ; idx-- )) ; do
		file=${files[idx]}
		enum_name=$(filename_to_enum "$file")
		echo "    $enum_name," >> $output_file
		save_last_enum=$enum_name
	done
	echo "};" >> $output_file
	echo -e "\n#define NEWAT_MAX $save_last_enum" >> $output_file
	echo -e "#endif /* HEADER_BLOCK_TYPE_H */" >> $output_file
}

create_enum_file
