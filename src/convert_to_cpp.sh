#!/bin/bash

# Script to rename all .c files to .cpp and update the Makefile accordingly

# Step 1: Create a backup of the Makefile
cp Makefile Makefile.bak

# Step 2: Rename all .c files to .cpp
for file in *.c; do
    if [ -f "$file" ]; then
        newname="${file%.c}.cpp"
        echo "Renaming $file to $newname"
        git mv "$file" "$newname"
    fi
done

# Step 3: Update the Makefile to use .cpp files
sed -i 's/\.c/.cpp/g' Makefile

# Step 4: Update object file rules
sed -i 's/%.o: %.cpp/%.o: %.cpp/g' Makefile

# Step 5: Update any other references to .c files
sed -i 's/C_FILES =/CPP_FILES =/g' Makefile

echo "Conversion completed. A backup of the Makefile has been saved as Makefile.bak"
echo "Run 'make clean && make' to test the build with the new .cpp files"
