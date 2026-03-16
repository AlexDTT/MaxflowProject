#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
INPUT_DIR="$SCRIPT_DIR/input"
OUTPUT_DIR="$SCRIPT_DIR/output"

cd "$PROJECT_ROOT" || exit 1

if [[ ! -x ./main ]]; then
    echo "Error: ./main binary not found. Run 'make' first."
    exit 1
fi

TOTAL=0
PASSED=0
FAILED=0

PASSED_OUTPUTS=()

echo "========================================"
echo "     Running Integration Tests"
echo "========================================"
echo ""

for input_file in "$INPUT_DIR"/dataset*.csv; do
    [[ -f "$input_file" ]] || continue
    basename_input=$(basename "$input_file")
    expected_file="$OUTPUT_DIR/$basename_input"
    if [[ ! -f "$expected_file" ]]; then
        echo "[SKIP] $basename_input - no expected output file"
        continue
    fi
    ((TOTAL++))
    output_name=$(grep -oP 'OutputFileName,\s*"\K[^"]+' "$input_file" 2>/dev/null || echo "output.csv")
    actual_output="$PROJECT_ROOT/$output_name"
    ./main -b "$input_file" >/dev/null 2>&1
    if [[ ! -f "$actual_output" ]]; then
        echo "[FAIL] $basename_input - output file not created"
        ((FAILED++))
        continue
    fi
    if diff -q --strip-trailing-cr "$actual_output" "$expected_file" >/dev/null 2>&1; then
        echo "[PASS] $basename_input"
        ((PASSED++))
        PASSED_OUTPUTS+=("$actual_output")
    else
        echo "[FAIL] $basename_input - output differs"
        echo "       Expected: $expected_file"
        echo "       Got:      $actual_output"
        echo "       Diff:"
        diff --strip-trailing-cr "$expected_file" "$actual_output" 2>&1 | head -20 | sed 's/^/       /'
        echo "       (Keeping $output_name for inspection)"
        ((FAILED++))
    fi
done

echo "========================================"
echo "       Integration Test Results"
echo "========================================"
echo "  Total:   $TOTAL"
echo "  Passed:  $PASSED"
echo "  Failed:  $FAILED"
echo "========================================"

for f in "${PASSED_OUTPUTS[@]}"; do
    rm -f "$f" 2>/dev/null
done

if [[ $FAILED -eq 0 ]]; then
    echo "All integration tests passed!"
    exit 0
else
    echo "Some tests failed. Failed test outputs kept for inspection."
    exit 1
fi