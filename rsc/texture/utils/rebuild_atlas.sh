#!/bin/bash

# This script is used to rebuild the texture atlas for the game.
# He take only one argument, the name of the NEW block texture to CHANGE.
# The script will move the old texture to the backup folder and copy the new texture to the texture folder.
# Then it will rebuild the texture atlas.

# Check if the argument is empty
if [ -z "$1" ]; then
	echo "You must provide the name of the new block texture to change."
	exit 1
fi

NAME="$1"

# Check if the backup folder exists
if [ ! -d "backup" ]; then
	mkdir backup
fi

# Check if the texture file exists
if [ ! -f "block_bmp/$NAME.bmp" ]; then
	echo "The texture file $NAME.bmp does not exist in block_bmp."
	exit 1
fi

# Check if the new given file exists
if [ ! -f "$NAME.bmp" ]; then
	echo "The file target $NAME.bmp ."
	exit 1
fi

# Move the old texture to the backup folder
mv "block_bmp/$NAME.bmp" "backup/$NAME.bmp"

# Copy the new texture to the texture folder
cp "$NAME.bmp" "block_bmp/"

# Rebuild the texture atlas
./utils/concat_file.sh block_bmp TESTatlas_block.bmp vertical