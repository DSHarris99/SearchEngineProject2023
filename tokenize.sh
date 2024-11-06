#!/bin/bash
# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 input_directory output_directory"
    exit 1
fi
echo 'Program written by Devin Harris'
temp_dir=$(mktemp -d)

# Compile all programs first
echo 'Compiling'
flex token.flex
g++ -o hw5 lex.yy.c -lfl

# Compile multiway merge program
g++ -o multiwaymerge multiwaymerge.cpp hashtable.cpp invertedFile.cpp accumulator.cpp

# Process documents within the input directory
input_dir="$1"
output_dir="$2"

file_count=0
echo "Running input documents through tokenizer, please wait..."
echo ' ' 
for input_file in "$input_dir"/*; do
    if [ -f "$input_file" ]; then
        #echo "Processing document: $input_file"
        # Extract the filename without the path
        filename=$(basename -- "$input_file")
        # Remove the file extension
        filename_noext="${filename%.*}"
        output_file="$output_dir/$filename_noext.txt"
        ./hw5 "$input_file" "$output_file"
		# Sort output files alphabetically after created
        sort -o "$output_file" "$output_file"

        ((file_count++))
    fi
done
echo "Finished tokenizing. Document hashtable entries from token.flex have been sorted alphabetically"
echo "and printed to output files for multiwaymerge.cpp to use..."
echo ' '
# After tokenization loop, run multiway merge
./multiwaymerge "$output_dir" 

# Clean up temporary directory
rm -rf "$temp_dir"

# Delete cumulative_total.txt after cpp and flex finish executing
if [ -f "cumulative_total.txt" ]; then
    rm "cumulative_total.txt"
fi

# Clear the entire $output directory
if [ -d "$output_dir" ]; then
    rm -r "$output_dir"/*
fi
rm -f hw5 multiwaymerge
echo ' '
echo "Finished processing $file_count documents"
