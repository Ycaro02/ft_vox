#!/bin/bash

# Concatenate all BMP files in the given directory into a single BMP file

source ../sh/color.sh

# Check if the number of arguments is correct
if [ $# -ne 3 ]; then
    display_color_msg ${YELLOW} "Usage: ./concat_file.sh <source_directory> <target_file> <horizontal|vertical>\n"
    exit 1
fi

SOURCE_DIR=${1}
TARGET_FILE=${2}
MODE=${3}

# Check if the source directory exists
if [ ! -d "${SOURCE_DIR}" ]; then
    display_color_msg ${RED} "Directory ${SOURCE_DIR} does not exist"
    exit 1
fi

if [ -f "${TARGET_FILE}" ]; then
    display_color_msg ${CYAN} "File ${TARGET_FILE} already exists, removing it..."
    rm "${TARGET_FILE}"
fi

# Determine the append mode based on user input
if [ "${MODE}" == "horizontal" ]; then
    APPEND_MODE="+append"
elif [ "${MODE}" == "vertical" ]; then
    APPEND_MODE="-append"
else
    display_color_msg ${RED} "Invalid mode: ${MODE}. Choose 'horizontal' or 'vertical'."
    exit 1
fi

# Concatenate all BMP files in the source directory into the target file
convert "${SOURCE_DIR}"/*.bmp ${APPEND_MODE} "${TARGET_FILE}"

#Number of BMP files in the source directory
NUM_FILES=$(ls -1q ${SOURCE_DIR}/*.bmp | wc -l)

display_color_msg ${LIGHT_MAGENTA} "Concatenated all BMP files in ${SOURCE_DIR} into ${TARGET_FILE}: ${NUM_FILES} files"