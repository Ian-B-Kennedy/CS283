#!/usr/bin/env bats

# --------------------------------------------------------------------
# Bats Test File - Additional Tests for RShell Server/Client
#
#   - Make sure you have your "dsh" (or your server/client executable)
#     in the same directory or adjust SERVER_CMD and CLIENT_CMD paths.
# --------------------------------------------------------------------

# Adjust commands/args as needed for your environment:
SERVER_CMD="./dsh -s 0.0.0.0:1234"
CLIENT_CMD="./dsh -c 127.0.0.1:1234"

setup() {
  # Start the server in the background, sending its output to a log.
  echo "Starting server on port 1234..."
  $SERVER_CMD &
  SERVER_PID=$!

  # Give the server a moment to start listening.
  sleep 1
}

teardown() {
  # Terminate the server if still running.
  echo "Stopping server with PID $SERVER_PID"
  kill $SERVER_PID 2>/dev/null || true
  wait $SERVER_PID 2>/dev/null || true
}

@test "Empty command line: no extra blank line" {
  run $CLIENT_CMD <<EOF

exit
EOF

  # We sent an empty line then exit. The main check is that the output
  # doesn’t produce repeated blank lines or error messages.
  # Inspect $output if you want to confirm no suspicious blank lines. 
  echo "Captured output:"
  echo "$output"

  [[ "$output" != *"Error parsing"* ]] || fail "Saw an 'Error parsing' message"
}

@test "Multiple commands in a single session" {
  run $CLIENT_CMD <<EOF
echo first
ls
echo second
exit
EOF

  echo "Output was:"
  echo "$output"

  # Check that both "first" and "second" appear, indicating multiple commands ran.
  [[ "$output" == *"first"* ]] || fail "Did not see 'first' in output"
  [[ "$output" == *"second"* ]] || fail "Did not see 'second' in output"
}

@test "Basic remote command: ls" {
  run $CLIENT_CMD <<EOF
ls
exit
EOF

  echo "Client output for 'ls':"
  echo "$output"

  [[ "$output" != *"error"* ]] || fail "Output contained 'error' unexpectedly."
}


@test "Pipeline test: echo Hello | tr [A-Z] [a-z]" {
  run $CLIENT_CMD <<EOF
echo Hello | tr [A-Z] [a-z]
exit
EOF

  # Expect output to be "hello"
  echo "Output was:"
  echo "$output"

  [[ "$output" == *"hello"* ]] || fail "Expected pipeline to produce 'hello'"
}

@test "Unknown command sets rc to 203" {
  run $CLIENT_CMD <<EOF
not_a_command
rc
exit
EOF

  # Show full output for debugging
  echo "Full output was:"
  echo "$output"

  # 1) Check for "Command not found in PATH" message
  [[ "$output" == *"Command not found in PATH"* ]] || fail "Did not see 'Command not found in PATH' message."

  # 2) Check that rc was "203"
  # Because your output is something like:
  #   rsh> not_a_command
  #   Command not found in PATH
  #   rsh> rc
  #   203
  #   rsh> exit
  [[ "$output" == *"203"* ]] || fail "Expected rc of 203, but did not see '203' in output."
}

@test "cd built-in changes directory" {
  # Create a test directory that we'll change into.
  mkdir -p test_dir

  run $CLIENT_CMD <<EOF
cd test_dir
pwd
exit
EOF

  echo "Full output from cd test:"
  echo "$output"

  # The server should print the new working directory from 'pwd'.
  # We expect it to end in "test_dir" or contain "test_dir" in the path.
  [[ "$output" == *"test_dir"* ]] || fail "Expected 'test_dir' in output after cd"

  rmdir test_dir
}

@test "Dragon command test" {
  run $CLIENT_CMD <<EOF
dragon
exit
EOF

  echo "Output was:"
  echo "$output"

  # If your dragon prints ASCII art that includes '@%%%%', check for that:
  [[ "$output" == *"@%%%%"* ]] || fail "Dragon ASCII art not found"
}

@test "Stop-server built-in gracefully ends server" {
  # This test checks that 'stop-server' shuts down the server. 
  # Then the client will exit or get disconnected.
  run $CLIENT_CMD <<EOF
stop-server
EOF

  echo "Output was:"
  echo "$output"

  # Typically no large errors expected, maybe a "Stopping server" message.
  [[ "$output" != *"error"* ]] || fail "Output contained an error"

  # Optionally wait a moment to ensure the server actually exited
  sleep 1

  # Attempt to see if the server is still up by connecting again.
  run $CLIENT_CMD <<EOF
echo ShouldFail
EOF
  # This should fail or produce no output if server is down.
  [[ "$output" == "" || "$output" == *"connect"* ]] || fail "Expected server to be down, but got: $output"
}
