#!/bin/bash

# Script to rename all .c files to .cpp

# Count how many files will be renamed
count=$(ls -1 *.c 2>/dev/null | wc -l)
echo "Found $count .c files to rename"

# Rename all .c files to .cpp
for file in *.c; do
    if [ -f "$file" ]; then
        newname="${file%.c}.cpp"
        echo "Renaming $file to $newname"
        mv "$file" "$newname"
    fi
done

echo "Conversion completed. $count files have been renamed."
echo "Run 'make clean && make' to test the build with the new .cpp files"
