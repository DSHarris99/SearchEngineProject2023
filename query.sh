#!/bin/bash

# Determine the directory where the script is located
script_dir="$(dirname "$0")"

# Change to the script's directory
cd "$script_dir" || exit

# only proceed if at least one word (argument) is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 word1 [word2] [word3] ..."
    exit 1
fi

# merge all arguments into a single string
query_string="$*"

# save the query to a file
echo "$query_string" > query.txt

# remove extra spaces and empty lines
sed -i 's/  */ /g' query.txt
sed -i '/^$/d' query.txt

# tokenize the query to temp file
flex token.flex
g++ -o tokenizer lex.yy.c -lfl
./tokenizer query.txt tokenizedquery.txt

# remove redundant frequency numbers from tokenizedquery.txt originating from token.flex
awk '{print $1}' tokenizedquery.txt > temp.txt && mv temp.txt tokenizedquery.txt

# Compile
g++ -o query query.cpp accumulator.cpp invertedFile.cpp
./query tokenizedquery.txt

# Cleanup
rm -f cumulative_total.txt query.txt tokenizedquery.txt query tokenizer 

echo 'Query processing completed.'
