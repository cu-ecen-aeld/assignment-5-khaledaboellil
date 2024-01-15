#!/bin/bash

writefile="$1"
writestr="$2"

if [ "$#" -ne 2 ]
then
echo"the arguments above were not specified"
exit 1 
fi

# Create the directory path if it doesn't exist
mkdir -p "$(dirname "$writefile")"

echo "$writestr" > "$writefile"
if [ "$?" -ne 0 ]; then
    echo "Error: Could not create the file $writefile"
    exit 1
fi
