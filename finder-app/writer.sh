#!/bin/bash

# Check if arguments are provided
if [ $# -ne 2 ]; then
    echo "Error: Please provide writefile and writestr as arguments"
    exit 1
fi

writefile="$1"
writestr="$2"

# Create the directory if it doesn't exist
mkdir -p "$(dirname "$writefile")"

# Write to the file
echo "$writestr" > "$writefile"

# Check if writing was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to write to $writefile"
    exit 1
fi

echo "Content written to $writefile"

