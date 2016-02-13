# Program summary:
	
> This project is the creation of a command shell called rshell.
> Main functions of the shell are to print a command prompt (e.g. $), 
> read in a command in one line, and execute each command. 

**Commands will take the form:**

	cmd       = executable [argumentList] [connector cmd]
	connector = || or && or ;

> The program is able to take in a limitless number of commands as
> long as they are chained together. 

**Example:** 

	$ ls -a
			
###### OR
			
	$ ls -a; echo hello; mkdir test
			
###### OR
			
	$ ls -a; echo hello && mkdir test || echo world; git status

**Basic Program Execution:**

- Execution of the command(s) will use the syscalls *fork*, *execvp*, and *waitpid*.
- A special built-in command of *exit* will exit the rshell. 
- Anything after a *#* will be ignored. 

# Known bugs:

> Below is a table of known bugs that we have recorded during our testings. 
> Note that this table does not reflect all possible errors available in the 
> code. These bugs are based on test cases we have thought of at the moment. 
> The current amount of bugs may or may not be the complete amount. However, 
> that does not mean that the code will function any less than the quality 
> desired. 

Bug Description | Terminal Output
--------------- | ---------------
bug 1 | bug 2
bug 3 | bug 4

**Note:** Any program that's bash specific will not work for the shell. 
