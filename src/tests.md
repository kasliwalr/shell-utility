## basic shell interface
- interactive and batch modes. one input arg => enters batch mode, 0 input arg => enter interactive mode, >= 2 input arg => exit w/o doing anything. print prompt in interactive mode

1. no arugment, print prompt and wait on same line
2. single argument, exit
3. more than one argument, print error


## basic command processing
- interactive mode
1. type each command and enter, should execute command and move to next line by printing prompt


## built-in vs generic command processing
- if any of exit/path/cd is entered, generic command logic should not be executed. 
1. `wish> path arg1 arg2`, generic command message not printed
2. `wish> exit arg1 arg2`, generic command message not printed
3. `wish> cd arg1`, generic command message not printed


## built-in command tests

### path command implementation 
0. print path to verify that default path is `/bin`
1. `> path ` - empties path list
2. `> path arg1` - overwrites path list with one item
3. `> path arg1 arg2` - overwrites path list with two items
4. `> external_command [arg1] [arg2]` - skips build-in command processing
5. if exec exits in two path, the one that comes first should be executed

### exit command implementation
0. improper exit invocation, should print error and continue
`wish> exit arg1 ....`
1. proper exit invocation, should exit interactive mode

### cd command implementation
1. `cd arg1`: should change to appropirate directory if arg1 is valid, else print error message from cd and continue
2. `cd`: should print error message
3. `cd arg1 arg2`: should print error message

## generic command processing

### w/o i/o redirection and parallelism
1. `wish> cmd arg1 arg2` should pass arguments, verified by correct running of commands, or error conditions
```
ls -a -l    # print output to screen
ls -blabla  # generate error message
```

### with i/o redirection w/o parallelism
1. parsing. give regular input command i.e. `command arg1 arg2...`. the redir_op should remain empty
2. parsing. give command with redirection operator i.e. `command arg1 arg2 > output`, output file name should be non-empty. repeat `command arg1 arg2` , output file should now be empty.
3. parsing. play with whitespace to verify that output file is still being read correctly
- `command arg1 arg2> output`
- `command arg1 arg2 >output`
- `command arg1 arg2>output`
4. parsing. bad syntax, should print error
- `command arg1 arg2>output1 output2`
- `command arg1 arg2>output1 > output2`
- `command arg1 arg2> >`
- `command arg1 arg2>>`
- `command arg1 arg2 > output1 >`
5. proper generation of output
- file not present, `command arg1 arg2> output` should create file and write to output to it
- file not present, `command badarg1>output` should create file and write error to it
- file present, `command arg1 arg2>output` should append to existing file
- file present, `command badarg1>output` should append to existing file
- file should be accessible by subsequent commands `cat file` should print contents

### parallel execution with and w/o i/o redirection 
1. run single command: ls -al > output: output should go to out file, parent waits for execution to complete 
2. run two commands: cmd1 arg1 > out & cmd2 arg2 > out2: cmd1 output goes to out, and cmd2 output goes to out2, parent waits for both executions to complete. this means parent prints its message only after children have printed their output 
3. run two commands: cmd1 arg1 > out & cmd2 arg2: cmd1 output goes to out, and cmd2 output goes to screen, 
4. run two commands: cmd1 arg1 & cmd2 arg2: cmd1 output goes to screen, and cmd2 output goes to screen
5. run two commands: cmd1 arg1 & cmd2 arg2 > out: cmd1 output goes to arg1, and cmd2 output goes to out
6. run two commands: bad syntax & cmd2 arg2: prints error for first bad syntax command, print 2nd comands output

## batch processing
1. should exit after running all the commands
- have 1 command in file of the form `cmd arg1`. should read one line
- have 2 command in file of the form. should read two lines 
```
cmd arg1
cmd arg2
```
2. have 1 command in file of the form. should read 1 line, and one empty line
```
cmd arg1
           # should not do anything
``` 
