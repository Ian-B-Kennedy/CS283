1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Using `fork` creates a new child process, which then calls `execvp` to replace its process image with that of the external command. If we called `execvp` directly in the shell process, it would replace the shell’s image, effectively terminating the shell. Forking allows the parent shell to remain running, wait for the child to finish, and continue accepting commands.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If `fork()` fails, it returns a negative value and no child process is created. In my implementation, I check for this condition (i.e., pid < 0), print an error message using `perror("fork")`, and then continue the loop without attempting to execute the command. This prevents further undefined behavior in the shell.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  The `execvp()` function searches for the executable in the directories specified by the PATH environment variable. The PATH variable contains a colon-separated list of directories that the system will search, in order, to locate the command if the command does not include an absolute or relative path.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The `wait()` call in the parent process pauses its execution until the child process terminates. This allows the parent to retrieve the exit status of the child and prevents the creation of zombie processes. Without waiting, the shell might spawn child processes that remain in a zombie state, and the shell would not know when the command has finished executing.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  `WEXITSTATUS(status)` extracts the exit code of the child process from the status value returned by `wait()` or `waitpid()`. This exit code tells us whether the child process terminated successfully or encountered an error. It is important for error handling and can be used to decide how the shell or subsequent commands should respond to the outcome of the child process.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In my implementation, when a quoted string is encountered, the parser treats everything inside the quotes as a single token. This means that spaces inside the quotes are preserved rather than being treated as delimiters. This handling is necessary because many commands require arguments that include spaces (such as file names or phrases) and splitting them incorrectly would change the meaning or behavior of the command.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: 
    >1. The parsing logic was refactored to use a single `cmd_buff_t` structure rather than managing a list of commands. This simplifies memory management and streamlines the overall control flow.  
    >2. The parser now carefully handles quoted strings by only removing quotes when a matching pair is found. This ensures that if a closing quote is missing, extra spaces are not inadvertently preserved.    
    >3. **Unexpected Challenges**:  
    >    -Managing pointer arithmetic and ensuring proper null-termination for both quoted and unquoted tokens was harder to implement than anticipated.  
    >    -Dealing with unmatched quotes required additional logic to prevent incorrect tokenization, which increased the overall complexity.  



8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals provide a mechanism for asynchronous notification in Linux. They allow the operating system or one process to interrupt another process to notify it of events (such as illegal memory access, termination requests, or timers expiring). Unlike other forms of IPC (such as pipes, sockets, or shared memory) which involve explicit data exchange and synchronization, signals are lightweight, carry minimal data (usually just the signal number), and are delivered asynchronously, meaning a process can be interrupted at nearly any point in its execution.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  
    >1. SIGKILL: This signal forcefully terminates a process and cannot be caught, blocked, or ignored. It is used when a process must be immediately terminated, for example, if it becomes unresponsive.
    >2. SIGTERM: This is a termination signal that requests a process to gracefully shut down. Processes can catch SIGTERM to perform cleanup operations before exiting. It is the default signal used by commands like kill (without specifying a signal).
    >3. SIGINT: Typically generated by the user pressing Ctrl+C in the terminal, SIGINT interrupts a process, allowing it to perform a graceful shutdown if desired. It is often used in interactive sessions to stop a running command.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives SIGSTOP, it is immediately suspended (paused) by the operating system. Unlike signals such as SIGINT, SIGSTOP cannot be caught, blocked, or ignored. This is because SIGSTOP is designed as a non-overridable mechanism to stop a process so that it can be inspected or managed by system utilities (such as debuggers or process management commands). This guarantees that a process can always be stopped regardless of its internal signal handling logic.