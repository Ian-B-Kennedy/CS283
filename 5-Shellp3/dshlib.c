#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
void trim_spaces(char *str) {
    char *start = str;
    char *end;

    // Skip leading spaces.
    while (*start && isspace((unsigned char)*start))
        start++;

    // If we skipped any characters, shift the trimmed string to the front.
    if (start != str)
        memmove(str, start, strlen(start) + 1);

    // Trim trailing spaces.
    end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
}

/*
 * build_cmd_buff:
 * Tokenizes a single command string (cmd_line) into a cmd_buff_t.
 * The tokenization splits the command into tokens based on whitespace,
 * handling quoted tokens. If an opening double quote is unmatched, it is
 * treated as literal and included in the token.
 *
 * The token pointers in argv refer into the _cmd_buffer.
 *
 * Returns OK on success or an error code if too many tokens or an oversized token is encountered.
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    // Save the command string in _cmd_buffer so that argv pointers remain valid.
    cmd_buff->_cmd_buffer = cmd_line;

    // Remove leading/trailing whitespace.
    trim_spaces(cmd_line);

    int argc = 0;
    char *p = cmd_line;
    while (*p) {
        // Skip any whitespace.
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0')
            break;

        char *token;
        if (*p == '"') {
            // We've encountered an opening quote.
            p++;  // Skip the opening quote.
            token = p;
            char *quote_end = strchr(p, '"');
            if (quote_end != NULL) {
                // Found a matching closing quote: terminate the token.
                *quote_end = '\0';
                p = quote_end + 1;
            } else {
                // No matching closing quote: include the opening quote as literal.
                token = p - 1;
                // Continue scanning until whitespace is encountered.
                while (*p && !isspace((unsigned char)*p))
                    p++;
                if (*p)
                    *p++ = '\0';
            }
        } else {
            // Process a non-quoted token.
            token = p;
            while (*p && !isspace((unsigned char)*p))
                p++;
            if (*p)
                *p++ = '\0';
        }

        // Check for too many tokens.
        if (argc >= CMD_ARGV_MAX)
            return ERR_CMD_ARGS_BAD;

        // Check if token size exceeds ARG_MAX.
        if (strlen(token) >= ARG_MAX)
            return ERR_CMD_OR_ARGS_TOO_BIG;

        cmd_buff->argv[argc++] = token;
    }
    cmd_buff->argc = argc;
    return OK;
}

/*
 * build_cmd_list:
 * Splits the overall command line (cmd_line) into individual commands using the PIPE_STRING delimiter.
 * For each command segment, it trims whitespace and then tokenizes it by calling build_cmd_buff.
 * The tokenized results are stored in the commands array of the command_list_t.
 *
 * Returns OK on success or an error code if too many commands or if an individual command fails to tokenize.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;

    // Use strtok to split by the pipe delimiter.
    char *token = strtok(cmd_line, PIPE_STRING);
    while (token != NULL) {
        if (clist->num >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;

        trim_spaces(token);

        // Tokenize this command segment.
        int ret = build_cmd_buff(token, &clist->commands[clist->num]);
        if (ret != OK)
            return ret;

        clist->num++;
        token = strtok(NULL, PIPE_STRING);
    }

    if (clist->num == 0)
        return WARN_NO_CMDS;

    return OK;
}

int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    int num_pipes = num_cmds - 1;
    int pipefds[2 * num_pipes];

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return ERR_MEMORY;
        }
    }

    pid_t pids[CMD_MAX];
    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return ERR_MEMORY;
        }
        if (pid == 0) {  /* Child process */
            if (i != 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2 - stdin");
                    exit(ERR_EXEC_CMD);
                }
            }
            if (i != num_cmds - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 - stdout");
                    exit(ERR_EXEC_CMD);
                }
            }
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }
            /* Ensure argv is NULL-terminated */
            clist->commands[i].argv[clist->commands[i].argc] = NULL;
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            if (errno == ENOENT)
                fprintf(stderr, "Command not found in PATH\n");
            else if (errno == EACCES)
                fprintf(stderr, "Permission denied\n");
            else
                fprintf(stderr, "CMD_ERR_EXECUTE\n");
            exit(ERR_EXEC_CMD);
        } else {
            pids[i] = pid;
        }
    }
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]);
    }
    int last_exit = 0;
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        /* The pipeline's exit code is defined as that of the last command */
        if (i == num_cmds - 1) {
            if (WIFEXITED(status))
                last_exit = WEXITSTATUS(status);
            else
                last_exit = -1;
        }
    }
    return last_exit;
}

int exec_local_cmd_loop() {
    int rc = 0;  /* Last command's exit status */
    char input_line[SH_CMD_MAX];

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("\n");
            break;
        }
        input_line[strcspn(input_line, "\n")] = '\0';
        if (strlen(input_line) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        if (strcmp(input_line, EXIT_CMD) == 0)
            break;

        command_list_t clist;
        int ret = build_cmd_list(input_line, &clist);
        if (ret == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        } else if (ret == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (ret != OK) {
            fprintf(stderr, "Error parsing command line: %d\n", ret);
            continue;
        }

        /* If there is only one command, check for built-in commands */
        if (clist.num == 1) {
            char *cmd_name = clist.commands[0].argv[0];
            if (strcmp(cmd_name, "dragon") == 0) {
                print_dragon();
                continue;
            }
            if (strcmp(cmd_name, "cd") == 0) {
                if (clist.commands[0].argc < 2)
                    continue;
                else {
                    if (chdir(clist.commands[0].argv[1]) != 0)
                        perror("cd");
                    continue;
                }
            }
            if (strcmp(cmd_name, "rc") == 0) {
                printf("%d\n", rc);
                continue;
            }
        }

        /* Execute external command or pipeline */
        if (clist.num == 1) {
            /* Single command (non-pipeline) */
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                continue;
            }
            if (pid == 0) {
                clist.commands[0].argv[clist.commands[0].argc] = NULL;
                execvp(clist.commands[0].argv[0], clist.commands[0].argv);
                if (errno == ENOENT)
                    fprintf(stderr, "Command not found in PATH\n");
                else if (errno == EACCES)
                    fprintf(stderr, "Permission denied\n");
                else
                    fprintf(stderr, "CMD_ERR_EXECUTE\n");
                exit(errno);
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status))
                    rc = WEXITSTATUS(status);
                else
                    rc = -1;
            }
        } else {
            /* Pipeline of commands */
            rc = execute_pipeline(&clist);
        }
    }
    return OK;
}