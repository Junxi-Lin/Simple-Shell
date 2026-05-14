# Simple Shell with Pipes (C)

## Overview
This project implements a custom Unix-like command-line shell in C. 
It supports process creation, command execution, input parsing, and 
inter-process communication using pipes.

## Features
- Command execution using fork() and execvp()
- Support for multiple command-line arguments
- Custom prompt support
- Process management with wait()
- Input parsing and tokenization
- Pipe support for chaining multiple commands
- Error handling and EOF handling

## Functionality
- Execute standard Linux commands (e.g., ls, ps, cat)
- Handle multiple arguments per command
- Support piping (e.g., `cat file.txt | wc -l`)
- Display child process ID and exit status
- Graceful termination using `exit` command

## Technologies
- C programming language
- Linux (Ubuntu)
- System calls: fork, execvp, wait, pipe, dup2
- Makefile build system

## My Contributions
- Implemented command parsing and argument tokenization
- Developed process creation and execution using fork/exec
- Implemented pipe functionality using pipe() and dup2()
- Handled input validation, errors, and EOF conditions
- Designed custom shell prompt support

## What I Learned
- Process creation and management in Unix systems
- Inter-process communication using pipes
- Command parsing and execution flow
- Low-level system programming in C
- Debugging multi-process programs

## How to Run
```bash
make
make run
