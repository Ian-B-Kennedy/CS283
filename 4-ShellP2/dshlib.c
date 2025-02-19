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
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // If we skipped any characters, shift the trimmed string to the front.
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // Trim trailing spaces.
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

static void parse_cmd_line(cmd_buff_t *cmd, char *line) {
    trim_spaces(line);

    int argc = 0;
    char *p = line;
    while (*p) {
        /* Skip whitespace outside of tokens */
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0')
            break;

        char *token;
        if (*p == '"') {
            /* We've encountered an opening quote.
             * Advance past it and look for the closing quote.
             */
            p++;  // Skip opening quote
            token = p;
            char *quote_end = strchr(p, '"');
            if (quote_end != NULL) {
                /* Matching closing quote found.
                 * Terminate the token at the closing quote.
                 */
                *quote_end = '\0';
                p = quote_end + 1;
            } else {
                /* No matching closing quote was found.
                 * In this case, we treat the initial double quote as a literal.
                 * Reset token to include the unmatched quote and process as an unquoted token.
                 */
                token = p - 1; // Include the unmatched quote
                while (*p && !isspace((unsigned char)*p))
                    p++;
                if (*p)
                    *p++ = '\0';
            }
        } else {
            /* Process a non-quoted token */
            token = p;
            while (*p && !isspace((unsigned char)*p))
                p++;
            if (*p)
                *p++ = '\0';
        }

        if (argc < CMD_ARGV_MAX)
            cmd->argv[argc++] = token;
    }
    cmd->argc = argc;
}


int exec_local_cmd_loop()
{
    int rc = 0;
    cmd_buff_t cmd;
    cmd._cmd_buffer = malloc(sizeof(char) * SH_CMD_MAX);

    if (!cmd._cmd_buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("%s", SH_PROMPT);

        if (fgets(cmd._cmd_buffer, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        /* Remove the trailing newline */
        cmd._cmd_buffer[strcspn(cmd._cmd_buffer, "\n")] = '\0';

        /* Warn if the command is empty */
        if (strlen(cmd._cmd_buffer) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        /* Parse the command line into the cmd_buff_t structure */
        parse_cmd_line(&cmd, cmd._cmd_buffer);


        if (cmd.argc == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        /* Handle built-in cd command */
        if (strcmp(cmd.argv[0], EXIT_CMD) == 0)
            break;

        if (strcmp(cmd.argv[0], "dragon") == 0) {
            print_dragon();
            continue;
        }
        
        if (strcmp(cmd.argv[0], "cd") == 0) {
            if (cmd.argc == 1) {
                /* No arguments: do nothing (unlike most Linux shells) */
                continue;
            } else {
                if (chdir(cmd.argv[1]) != 0)
                    perror("cd");
                continue;
            }
        }

        if (strcmp(cmd.argv[0], "rc") == 0) {
            printf("%d\n", rc);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            /* Child process */
            cmd.argv[cmd.argc] = NULL;  // Ensure argv is NULL-terminated
            execvp(cmd.argv[0], cmd.argv);
            /* If execvp returns, an error occurred */
            if (errno == ENOENT) {
                fprintf(stderr, "Command not found in PATH\n");
            } else if (errno == EACCES) {
                fprintf(stderr, "Permission denied\n");
            } else {
                fprintf(stderr, "CMD_ERR_EXECUTE\n");
            }
            exit(errno);  // Return the errno value as the exit code
        } else {
            /* Parent process: wait for the child process to finish */
            int status;
            pid_t wpid;
            while ((wpid = waitpid(pid, &status, 0)) == -1 && errno == EINTR)
                ;
            if (wpid == -1) {
                perror("waitpid");
            } else {
                if (WIFEXITED(status)) {
                    rc = WEXITSTATUS(status);
                } else {
                    rc = -1;  // If the child did not exit normally
                }
            }
        }
    }

    free(cmd._cmd_buffer);
    return OK;
}