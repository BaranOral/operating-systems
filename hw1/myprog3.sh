#!/bin/bash

#take input when bash program called.
directory=$1

if [ -z "{$directory}" ] #if input is empty, set directory to current directory.
then
	directory=$(pwd)
else 
	#check entered input is a valid directory or not. If it is valid find all zero length file and delete them. If it is not valid directory, give error and exit with 1 flag.
	if [ -d $directory ]
	then
		zeroFilesCount=$(find $directory -size 0 | wc -l )
		find $directory -size 0 -delete
		echo "$zeroFilesCount zero-length files are removed from the directory: $directory"
	
	else
		echo "Error occurs!"
		exit 1
	fi	 	
fi
