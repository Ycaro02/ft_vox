#!/bin/bash

source ../sh/color.sh

# This script is used change to add index in a name of a file
# Usage: ./indexFile.sh ${1} ${2} ${3}

# Example in directory rsc, foo.png, bar.png, baz.png
# ./indexFile.sh rsc test 0
# Will create with the command mv test/0_foo.png, test/1_bar.png, test/2_baz.png

# Check if the number of arguments is correct
if [ $# -ne 3 ]; then
	display_color_msg ${YELLOW} "Usage: ./indexFile.sh ${1} ${2} ${3}\n"
	display_color_msg ${YELLOW} "\t$<1> is the directory containing files\n"
	display_color_msg ${YELLOW} "\t<2> is the directory to store the index file\n"
	display_color_msg ${YELLOW} "\t<3> is the index to start\n"
	exit 1
fi

SOURCE_DIR=${1}
TARGET_DIR=${2}
INDEX=${3}

# Check if the source directory exists
if [ ! -d ${SOURCE_DIR} ]; then
	display_color_msg ${RED} "Directory ${SOURCE_DIR} does not exist"
	exit 1
fi

# Check if the target directory exists
if [ ! -d ${TARGET_DIR} ]; then
	display_color_msg ${CYAN} "Directory ${TARGET_DIR} does not exist, creating it..."
	mkdir ${TARGET_DIR}
fi

# Apply the conversion on all files in the source directory
for file in ${SOURCE_DIR}/*
do
	filename=$(basename ${file})
	display_color_msg ${LIGHT_MAGENTA} "Copy ${file} to ${TARGET_DIR}/${INDEX}_${filename}"
	cp ${file} ${TARGET_DIR}/${INDEX}_${filename}
	INDEX=$((INDEX+1))
done