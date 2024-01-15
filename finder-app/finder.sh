#!/bin/bash
filedir="$1"
searchstr="$2"

if [ "$#" -ne 2 ]
then
echo "Two arguments are required - filesdir and searchstr"
exit 1
fi

if [ ! -d "$filedir" ]
then
echo "$filedirs does not represent a directory on the filesystem "
exit 1
fi
numoffile=$(find "$filedir" -type f | wc -l)
matchingline=$(grep -r "$searchstr" "$filedir" | wc -l)
echo "The number of files are $numoffile and the number of matching lines are $matchingline"
