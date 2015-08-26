#!/bin/bash
SOURCES="$(find -L src -name "*.h" -o -name "*.c" -o -name "*.cpp")"

LICENCE="$(cat Licence_header.txt)"

for SOURCE in $SOURCES
do
	if [ "$(head -1 $SOURCE)" != "/*" ]
	then
		echo "$LICENCE" > "tmp.txt"
		echo "$(cat $SOURCE)" >> "tmp.txt"
		echo "Licence $SOURCE"
		mv "tmp.txt" "$SOURCE"
	else 
		count=0
		found=0
		while read line
		do
			let count=count+1
			if [ "$line" == "*/" ]
			then
				found=1
				break
			fi
		done < "$SOURCE"
		if [ $found -ne 1 ]
		then
			echo "Error: $SOURCE"
		else
			echo "$LICENCE" > "tmp.txt"
			awk "FNR>$count" $SOURCE >> "tmp.txt"
			if ! diff -q "tmp.txt" "$SOURCE"
			then
				mv "tmp.txt" "$SOURCE"
			fi
		fi
	fi
done
