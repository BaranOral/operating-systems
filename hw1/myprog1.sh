#!/bin/bash

filename=$1

echo "Welcome to STARS!."

#reading line loop
while read line
do
	if [ -z $line ]# if line is empty, print error message.
	then
		echo "Input cannot be empty, I couldn't printed it!"
	
	else
	#check it is positive or negative. If it is positive print * same amount as number.s
		if [ $line -gt 0 ];
		then
			
			for (( counter=$line; counter>0; counter--)) 
			do
				printf "*" 
			done
			printf "\n"
		
		else
			echo "It is negative integer, I couldn't printed it!"
			
		fi
	fi
done<$filename
