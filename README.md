Custom Shell Program in C
This repository contains a custom shell program implemented in C. The shell provides functionalities similar to common Unix-like shell environments, including command execution, variable substitution, background processes, pipes, and signal handling.

Features
Command execution: Execute user-entered commands with support for arguments.
Variable substitution: Define and use variables, replacing $var with their meanings.
Background processes: Run processes in the background using the & symbol.
Pipes: Chain commands together using pipes for input and output redirection.
Signal handling: Handle signals such as SIGCHLD for child process termination and SIGTSTP for suspending processes.
Input/output redirection: Redirect input and output to/from files using >.
Usage
Compile the source code using your preferred C compiler (e.g., gcc).

gcc -o custom_shell custom_shell.c
Run the compiled executable.
./custom_shell
Enter commands and interact with the custom shell.

Examples:

Execute a command:
$ ls -l

Variable substitution:
$ VAR_NAME=value
$ echo $VAR_NAME

Background process:
$ sleep 10 &

Pipe commands:
$ ls -l | grep ".txt"

Input/output redirection:
$ ls -l > output.txt
