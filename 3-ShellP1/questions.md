1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  `fgets()` is a good choice because it safely reads user input, including spaces, without causing buffer overflows. Unlike `scanf()`, which stops at whitespace, `fgets()` captures the entire line, making it ideal for handling full command-line inputs. Additionally, it allows us to specify a maximum buffer size, preventing excess input from overwriting memory. Finally, `fgets()` sets the null terminator, ensuring the string is properly formatted before processing.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using `malloc()` allows dynamic memory allocation, making the shell more flexible in handling varying input lengths. If we used a fixed-size array, we might either waste memory with overly large buffers or risk truncating commands if the buffer is too small. Dynamic allocation ensures we allocate only the needed memory and can reallocate if necessary, improving efficiency and scalability.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming spaces ensures that commands are correctly parsed and executed. If we don’t trim, leading spaces could cause incorrect command matching, and trailing spaces might interfere with argument parsing. For example, `ls` and ` ls` may not be treated the same if extra spaces exist. Untrimmed spaces could also cause unexpected behavior in command execution, leading to failed lookups or incorrect arguments.


4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  
    >1. **Redirect output to a file** (`ls > output.txt`):  
    >- Saves command output to a file instead of displaying it.  
    >- **Challenge:** Handling file permissions and correctly parsing the redirection operator (`>`).
    >
    >2. **Append output to a file** (`echo "Hello" >> log.txt`):  
    >- Appends to a file instead of overwriting it.  
    >- **Challenge:** Differentiating between `>` (overwrite) and `>>` (append), and handling file descriptors properly.
    >
    >3. **Redirect input from a file** (`sort < data.txt`):  
    >- Feeds file content as input to a command.  
    >- **Challenge:** Opening the file and correctly replacing `STDIN` with the file descriptor.


- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  
    >Redirection changes where input or output comes from or goes (e.g., a file), while piping (`|`) connects commands by sending one’s output as the other’s input.  
    >
    >For example:  
    >- `ls | grep txt` passes `ls` output directly to `grep`, enabling data processing between commands.  
    >- `ls > file.txt` saves the output to a file instead.  
    >
    >Pipes involve inter-process communication, while redirection deals with file streams.


- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  
    >Keeping STDERR separate ensures that errors don’t mix with normal output, allowing clean parsing. If errors were combined with output, scripts might misinterpret messages, leading to incorrect processing.  
    >
    >For example, `grep pattern file` should only return matches, not error messages if the file doesn’t exist. This separation helps in debugging and proper handling of command outputs.


- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  
    >Our shell should check return values and use `perror()` or custom messages for errors. We should also provide a way to merge STDOUT and STDERR using `2>&1`, like:  
    >
    >```sh
    >command > output.txt 2>&1
