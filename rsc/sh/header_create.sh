#!/bin/bash

source rsc/sh/color.sh

# Create header C file with $1 name and HEADER_$1_H protect define
NAME=${1}
UPPER_NAME=""

if [ -z ${NAME} ]; then
    display_color_msg ${RED} "Usage: header_create.sh <file_name>, file_name is required, without '.h' extension"
    exit 1
fi

string_to_uppercase() {
    local str="${1}"
    UPPER_NAME=$(echo ${str} | tr '[:lower:]' '[:upper:]')
}

create_C_header_file() {
    string_to_uppercase ${NAME}
    local hdr_protect=HEADER_${UPPER_NAME}_H
    printf "#ifndef ${hdr_protect}\n#define ${hdr_protect}\n\n\n\n#endif /* ${hdr_protect} */\n" > ${NAME}.h
}

create_C_header_file

