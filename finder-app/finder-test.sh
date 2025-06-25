#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data

# Use the correct path for the username file
username=$(cat /etc/finder-app/conf/username.txt)

if [ $# -lt 3 ]
then
    echo "Using default value ${WRITESTR} for string to write"
    if [ $# -lt 1 ]
    then
        echo "Using default value ${NUMFILES} for number of files to write"
    else
        NUMFILES=$1
    fi    
else
    NUMFILES=$1
    WRITESTR=$2
    WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# Use the correct path for the assignment file
assignment=$(cat /etc/finder-app/conf/assignment.txt)

if [ "$assignment" != 'assignment1' ]
then
    mkdir -p "$WRITEDIR"

    # The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple arguments.
    # The quotes signify that the entire string in WRITEDIR is a single string.
    # This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
    if [ -d "$WRITEDIR" ]
    then
        echo "$WRITEDIR created"
    else
        exit 1
    fi
fi
echo "Removing the old writer utility and compiling as a native application"
make clean
make

for i in $( seq 1 $NUMFILES)
do
    writer.sh "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

# Capture the output of the finder command and write it to /tmp/assignment4-result.txt
finder.sh "$WRITEDIR" "$WRITESTR" > /tmp/assignment4-result.txt

# Read the output back from the file
OUTPUTSTRING=$(cat /tmp/assignment4-result.txt)

# Remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo "${OUTPUTSTRING}" | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
    echo "success"
    exit 0
else
    echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
    exit 1
fi
