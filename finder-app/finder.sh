#!/bin/bash

# Check if arguments are provided
if [ $# -ne 2 ]; then
    echo "Error: Please provide filesdir and searchstr as arguments"
    exit 1
fi

filesdir="$1"
searchstr="$2"

# Check if filesdir is a directory
if [ ! -d "$filesdir" ]; then
    echo "Error: $filesdir is not a directory"
    exit 1
fi

# Function to search for the string in files
search_files() {
    local files="$1"
    local str="$2"
    local count=0

    while IFS= read -r file; do
        # Search for the string in the file and count matching lines
        match_count=$(grep -c "$str" "$file")
        count=$((count + match_count))
    done < <(find "$files" -type f)

    echo "$count"
}

# Get the number of files in the directory and subdirectories
file_count=$(find "$filesdir" -type f | wc -l)

# Get the number of matching lines
matching_lines=$(search_files "$filesdir" "$searchstr")

echo "The number of files are $file_count and the number of matching lines are $matching_lines"

