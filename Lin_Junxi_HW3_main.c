/**************************************************************
* Class::  CSC-415-02 Spring 2026
* Name:: Junxi Lin
* Student ID:: 923696927
* GitHub-Name:: Junxi-Lin
* Project:: Assignment 3 – Simple Shell with Pipes
*
* File::<Lin_Junxi_HW3_main.c>
*
* Description::
* Emulate the functionality of a simple shell, including pipes
**************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_SIZE 85
#define MAX_CMDS 32

/*  Tokenizer */

char **tokenize_command(char *cmd)
{
    int token_count = 0;
    char **tokens = NULL;             // Build argv-style argument list

    char *token = strtok(cmd, " \t"); // Split command on whitespace
    while (token != NULL)
    {
        // Based on the unknown counting dynamic expansion parameter list
        char **new_tokens =
            realloc(tokens, (token_count + 1) * sizeof(char *));
        if (new_tokens == NULL)
        {
            free(tokens);             // Prevent leak if fail
            return NULL;              // Signal parse failure to caller
        }

        tokens = new_tokens;          // Update pointer after realloc
        tokens[token_count++] = token; // Store current argument
        token = strtok(NULL, " \t");   // Continue parsing command
    }

    // Add an extra empty space to accommodate the necessary null terminator
    char **new_tokens =
        realloc(tokens, (token_count + 1) * sizeof(char *));
    if (new_tokens == NULL)
    {
        free(tokens);
        return NULL;
    }

    tokens = new_tokens;
    tokens[token_count] = NULL;       // execvp require argv end with NULL value

    return tokens;                    // Call execvp
}

/*  Print Status */
void print_pid(pid_t pid, int status)
{
    int code = 1;

    if (WIFEXITED(status))
        code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) // terminated by signal.
        code = 128 + WTERMSIG(status);

    printf("Child %d finished with status: %d\n", (int)pid, code);
}

/* Main */

/* Entry point of the shell. */

int main(int argc, char *argv[])
{
    //Allow setting of optional custom prompt messages through command-line. 
    const char *prompt = (argc > 1) ? argv[1] : "> ";

    while (1)
    {
        char input[MAX_INPUT_SIZE];

        printf("%s", prompt);
        fflush(stdout);  // Make sure prompt display immediately.

        // this input bounded by MAX_INPUT_SIZE.
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            if (feof(stdin))
                break;

            perror("fgets");
            exit(1);
        }

        //Trim trailing newline to simplify comparison and parsing.
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
            input[len - 1] = '\0';

        // Ignore empty input to avoid unnecessary process.
        if (input[0] == '\0')
        {
            printf("Error: empty command\n");
            continue;
        }

        // exit command by shell level.
        if (strcmp(input, "exit") == 0)
            break;

        /* -------- Split by pipe -------- */

        char *cmds[MAX_CMDS];
        int cmd_count = 0;

        char *saveptr = NULL;
        char *cmd = strtok_r(input, "|", &saveptr);

        while (cmd != NULL && cmd_count < MAX_CMDS)
        {
            while (*cmd == ' ' || *cmd == '\t')
                cmd++;

            if (*cmd == '\0') // Detect invalid cases like (||).
            {
                printf("Error: empty command in pipeline\n");
                cmd_count = 0;
                break;
            }

            cmds[cmd_count++] = cmd; // Store this pipeline segment for later execution.

            cmd = strtok_r(NULL, "|", &saveptr);  // Continue scanning remaining segments in the same input line.
        }

        if (cmd_count == 0)
            continue;

        /* -------- Execute commands -------- */

        pid_t pids[MAX_CMDS]; // Record child PIDs for later.
        int pid_count = 0;
        int prev_read = -1;

        for (int i = 0; i < cmd_count; i++)
        {
            int fd[2] = {-1, -1};  // Pipe for this stage.
            int need_pipe = (i < cmd_count - 1);  // not pipe out for last command.

            if (need_pipe)
            {
                if (pipe(fd) == -1)  // Create a new pipe to counitnue carry this command's output forward.
                {
                    perror("pipe");
                    exit(1);
                }
            }
             // Convert this command segment into argv for execvp.
            char **tokens = tokenize_command(cmds[i]);
            if (tokens == NULL || tokens[0] == NULL)
            {
                free(tokens);
                continue;
            }

            pid_t pid = fork();

            if (pid < 0)
            {
                perror("fork");
                exit(1);
            }

            if (pid == 0)
            {
                /* child */
                // If this is not first command, read from previous pipe.
                if (prev_read != -1)
                {
                     // If not last , write next pipe.
                    dup2(prev_read, STDIN_FILENO);
                }

                if (need_pipe)
                {
                    dup2(fd[1], STDOUT_FILENO);
                }

                if (prev_read != -1)
                    close(prev_read);

                if (need_pipe)
                {
                    close(fd[0]);
                    close(fd[1]);
                }
                
                // Replace this process to the target program.
                execvp(tokens[0], tokens);

                perror("execvp");
                exit(127);
            }

            /* parent */

            pids[pid_count++] = pid;  // Track child for later 
            free(tokens);

            if (prev_read != -1)  //  Close previous pipe read end after it has been handed off.
                close(prev_read);

            if (need_pipe)
            {
                close(fd[1]);
                prev_read = fd[0];
            }
            else
            {
                if (fd[0] != -1)
                    close(fd[0]);
                prev_read = -1;
            }
        }
  
        if (prev_read != -1) // Make sure no dangling read remains.
            close(prev_read);


        for (int i = 0; i < pid_count; i++)
        {
            int status;
            pid_t w = waitpid(pids[i], &status, 0);
            if (w > 0)
                print_pid(w, status);
        }
    }

    return 0;
}
