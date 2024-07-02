# Shell program

## Introduction

This shell program was written in C. It was completed as part of coursework in [Introduction to Operating Systems](http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/) at U. Wisconsin. I decided to include it as one of my show case projects because I enjoyed it very much. My reasons are as follows

1. It was fairly challenging given that it was in C 
2. It required some fundamental understanding of how processes work and use that knowledge to exercise the Linux API suitably. It is always nice to learn more about the Linux API
3. It involved parsing and although very elementary it sparked my interest in learning more about parsers


The shell program specifications are on the course [project repository](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell). There is no design document, however, the [source code](./src/wish.c) is sufficiently commented. 


## Prerequisites
You would need a bazel installation on your machine. To install bazel use bazelisk, this is the most portable way for Mac/Linux/Windows

## Installation

Since `wish` uses the POSIX API and no libraries except the GNU C library, it should run on any Linux machine. The installation instruction are provided below

1. Clone the repo and perform the following steps 
```
> git clone https://github.com/kasliwalr/intro_os_course.git
> cd  intro_os_course/project_assignments/p2a 
> mkdir bin  # create the 'bin' directory to hold the executable
> tree -L 2
.
├── LICENSE
├── MODULE.bazel
├── README.md
├── WORKSPACE
└── src
    ├── BUILD.bazel
    ├── tests.md
    └── wish.c

6 directories, 7 files
> bazel run //src:wish --cxxopt='-std=c++11'  # builds and run the application

> bazel build //src:wish --cxxopt='-std=c++17' # build only, and then invoke as described in #demo
```

## Demo

The `wish` shell program is built to specifications mentioned above. I will demonstrate the important features below


### Starting the shell
Your can start the shell in interactive or batch mode. As shown below, simply typing the shell command name, `wish`, starts the shell process in interactive mode. In this mode, the user can type one or more commands that they wish to execute 

```
> wish
wish>       # now in interactive mode
wish> ls
script.txt  wish
wish> ls -l
total 28
-rw-rw-r-- 1 rk rk    14 Jan 20 12:49 script.txt
-rwxrwxrwx 1 rk rk 23480 Sep 24 04:05 wish
```

In the batch mode, the user needs to provide a text file which contains a list of commands. The format of the file is same as what one would type on the terminal, for example, the following is a snippet of a shell script that can be provided to `wish`
```
# shell script
ls
ls -l
who
```
You can run the script as follows
```
> wish shell_script.txt
script.txt  wish                   # output of ls
total 28                           # output of ls -l
-rw-rw-r-- 1 rk rk    14 Jan 20 12:49 script.txt
-rwxrwxrwx 1 rk rk 23480 Sep 24 04:05 wish
An error has occurred              # output of who
```

### Setting the path
The `wish` shell uses `/bin` on your Linux machine to search for executable names that you type in. If it cannot find a name on this path, it will output an error. This is what happened when I ran the `who` command from the shell script. The `wish` outputted "An error has occured". On my system, `who` is located at `/usr/bin`. You can make `wish` more useful by specifying all the path names that you want `wish` to use, like so

```
wish> who
An error has occurred
wish> path /bin /usr/bin
wish> who
rk       tty7         2019-12-21 15:00 (:0)
wish> 
```

Here, `wish` looked for `who` at both locations, and found it at `/usr/bin/who`

### Redirecting output
The command syntax that `wish` is capable of taking is not just command names and their options. One can also redirect the output like so

```
wish> ls -l 
total 28
-rw-rw-r-- 1 rk rk    14 Jan 20 12:49 script.txt
-rwxrwxrwx 1 rk rk 23480 Sep 24 04:05 wish
wish> ls -l > output_ls.txt
```

The `output_ls.txt` looks like so

```
total 28
-rw-rw-rw- 1 rk rk     0 Jan 20 13:07 output_ls.txt
-rw-rw-r-- 1 rk rk    14 Jan 20 12:49 script.txt
-rwxrwxrwx 1 rk rk 23480 Sep 24 04:05 wish
```

### Running commands in parallel
The command syntax also allows to run multiple commands in parallel by inserting an `&` between them

```
sh> path /bin /usr/bin
wish> ls & who & which who
output_ls.txt  script.txt  sh_output_ls.txt  wish   # output of `ls`
rk       tty7         2019-12-21 15:00 (:0)         # output of `who`
/usr/bin/who                                        # output of `which who`
wish> 
```

There is a bug in this functionality. If one has path set to `/bin` which does not contain `who`, the output should contain "An error has occured" when `who` is processed. Instead, it prints output of `ls` twice. It is being worked upon. 
	

## Summary
So, in summary, you can run stuff in both batch and interactive mode. You can experiment quite a bit in interactive mode by running things in parallel, redirecting their output or a combination thereof. Try and enjoy!




