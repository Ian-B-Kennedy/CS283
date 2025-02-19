#!/usr/bin/env bats

#-----------------------------------------------------------
# Student Tests for dsh Shell (Heredoc Input)
#
# These tests verify functionality for built-in commands,
# external command execution, proper handling of quotes,
# the special "dragon" command, and extra credit return code handling.
#-----------------------------------------------------------

setup() {
    # Save the current working directory to restore later.
    ORIG_DIR="$(pwd)"
}

teardown() {
    # Restore the current working directory after each test.
    cd "$ORIG_DIR" 2>/dev/null
}

@test "cd with no arguments does not change working directory" {
    run "./dsh" <<EOF
pwd
cd
pwd
exit
EOF
    # Extract lines starting with '/' (assuming pwd outputs an absolute path).
    dir_lines=$(echo "$output" | grep '^/')
    first_dir=$(echo "$dir_lines" | head -n1)
    second_dir=$(echo "$dir_lines" | tail -n1)
    [ "$first_dir" = "$second_dir" ] || { echo "Expected same working directory but got first: '$first_dir' and second: '$second_dir'. Full output: $output"; false; }
}

@test "cd with an argument changes working directory" {
    tmp_dir="temp_test_dir"
    mkdir -p "$tmp_dir"
    run "./dsh" <<EOF
cd $tmp_dir
pwd
exit
EOF
    expected_dir="${ORIG_DIR}/${tmp_dir}"
    out_dir=$(echo "$output" | grep '^/')
    [ "$out_dir" = "$expected_dir" ] || { echo "Expected working directory to be '$expected_dir' but got '$out_dir'. Full output: $output"; false; }
    rm -rf "$tmp_dir"
}

@test "external command execution works (echo)" {
    run "./dsh" <<EOF
echo hello
exit
EOF
    echo "$output" | grep -q "hello" || { echo "Expected output to contain 'hello'. Full output: $output"; false; }
}

@test "quoted string handling preserves spaces" {
    run "./dsh" <<EOF
echo "hello   world"
exit
EOF
    echo "$output" | grep -q "hello   world" || { echo "Expected output to contain 'hello   world'. Full output: $output"; false; }
}

@test "dragon command prints ASCII art" {
    run "./dsh" <<EOF
dragon
exit
EOF
  echo "$output" | grep -q "@%%%%" || { echo "Expected output to contain '@%%%%'. Full output: $output"; false; }
}

@test "External command not found returns proper error code and message" {
    run "./dsh" <<EOF
not_exists
rc
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="CommandnotfoundinPATHdsh2>dsh2>dsh2>2dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]
}

@test "External command with permission denied returns proper error code and message" {
    tmp_file="temp_nonexec.sh"
    echo "#!/bin/sh" > "$tmp_file"
    chmod 644 "$tmp_file"  # Remove execute permissions.
    run "./dsh" <<EOF
./$tmp_file
rc
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="Permissiondenieddsh2>dsh2>dsh2>13dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]
    rm -f "$tmp_file"
}

@test "Successful external command returns 0" {
    run "./dsh" <<EOF
echo hello
rc
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="hellodsh2>dsh2>0dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]
}

