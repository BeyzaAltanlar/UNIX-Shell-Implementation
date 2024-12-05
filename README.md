# UNIX-Shell-Implementation
Project Explanation :

you are expected to develop an interface program for a UNIX Shell using the C programming language. The name of this interface should be "osh". The osh program should take and execute shell commands. Additionally, it should support features like input/output redirection (<, >), inter-process communication via pipes (|), and managing command history.

Requirements:

Your program should run in an infinite loop and wait for user input. In each iteration, it should display "osh->" on the screen, and the user should enter a command to the right of this prompt.
Commands provided as input to the program should be executed by creating a child process.
The execvp() function should be used to execute the commands.
Inter-process communication should be handled using pipe(). More than two commands may be chained together.
Every executed command should be added to the command history. To manage the command history, you can use a LinkedList or an Array. Commands invoked from the history using "!!" should also be re-added to the command history.
Except for the following headers: <stdio.h>, <stdlib.h>, <sys/types.h>, and <string.h>, you are not allowed to use any additional libraries.
Example Usage:
Your osh program should function as follows:

bash
~$ ./osh  
osh -> cat test.txt  
# The contents of the file test.txt should be printed to the screen.  

osh -> ls -l | head -2  
# The first two lines of the output of the `ls -l` command should be printed to the screen.  

osh -> ls > out.txt  
# The output of the `ls` command should be written to the file out.txt.  

osh -> sort -r < sort_test.txt  
# The contents of sort_test.txt should be printed to the screen, sorted in reverse line order.  

osh -> !! 0  
# Re-executes the last executed command.  

osh -> !! 1  
# Re-executes the command before the last executed command.  

osh -> !! 2  
# Re-executes the command executed two commands ago.  

osh -> *exit*  
# Terminates the program.  
