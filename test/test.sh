#!/bin/bash

# Track if any failures occurred
failed=0

# Find all files that don't start with "C-"
for file in *.dts; do
    # Skip if it's a directory or starts with "C-"
    if [[ -d "$file" ]] || [[ "$file" == C-* ]]; then
        continue
    fi

    # Construct the expected "C-" prefixed filename
    expected_file="C-$file"

    # Skip if the corresponding "C-" file doesn't exist
    if [[ ! -f "$expected_file" ]]; then
        continue
    fi

    # Run ../dt-format with the file as argument and capture output
    output=$(../dt-format "$file")

    # Read the content of the "C-" prefixed file
    expected_content=$(cat "$expected_file")

    # Compare the output with the expected content
    if [[ "$output" != "$expected_content" ]]; then
        echo "Miss match $file"
	failed=1
    fi
done

exit $failed
