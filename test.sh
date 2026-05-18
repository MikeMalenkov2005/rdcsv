#!/bin/bash

# Build the application
./build.sh || { echo "Build failed"; exit 1; }

total=0
passed=0
failed=0

# Iterate over all CSV files in test/t*.csv
for csv in test/t*.csv; do
    base=$(basename "$csv" .csv)
    N=${base#t}
    expected_exit_file="test/expected/e${N}.txt"
    expected_output_file="test/expected/o${N}.txt"

    if [[ ! -f "$expected_exit_file" || ! -f "$expected_output_file" ]]; then
        echo "Missing expected files for test $N, skipping"
        continue
    fi

    actual_output=$(mktemp)

    # Run the executable, capture combined stdout+stderr
    ./rdcsv "$csv" > "$actual_output" 2>&1
    exit_code=$?

    # Read expected exit code (strip any whitespace)
    expected_exit=$(tr -d '[:space:]' < "$expected_exit_file")

    if [[ $exit_code -eq $expected_exit ]]; then
        if diff -q "$actual_output" "$expected_output_file" > /dev/null 2>&1; then
            echo "Test $N PASSED"
            ((passed++))
        else
            echo "Test $N FAILED: output mismatch"
            ((failed++))
        fi
    else
        echo "Test $N FAILED: exit code mismatch (got $exit_code, expected $expected_exit)"
        ((failed++))
    fi

    rm -f "$actual_output"
    ((total++))
done

echo "Summary: $passed passed, $failed failed out of $total"
exit $((failed > 0 ? 1 : 0))

