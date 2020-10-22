#define _BSD_SOURCE
#define _XOPEN_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/*****************************************************************************************
Data Structures
*****************************************************************************************/
// data structure for storing command name, its arguments and any redirection sinks 
typedef struct command_data
{
  char* command_name;
  char** arglist;
  char* redir_op;
  int num_args;
  char bad_syntax;
} command_data;

// data structure for storing pathname list. starts with some initial size and resizes when needed
typedef struct path_list
{
  char** paths;
  int sz;
  int used;
} path_list;


/*********************************************************************************
Command Handling
**********************************************************************************/
command_data parse_command(char*);
void run_command(char*);
void free_command(command_data*);

/**********************************************************************************
Search Path Management
**********************************************************************************/
void clearpath(path_list *);
void initialize_path(path_list*, char* pathname);
void set_path(path_list*, char** path_names, int);
void find_command(char*, path_list*, char* command_path); 
path_list search_path;     // global search path 

/**********************************************************************************
Error Handling
**********************************************************************************/
void error();

/********************************************************
Utilities
********************************************************/
void print_command(command_data);
void printpath(path_list*);


int main(int argc, char* argv[])
{

  if (argc > 2)         // only allows syntax indicating batch or interactive mode 
  {
    error();
    exit(1);
  }
 
  // set default path search directory 
  initialize_path(&search_path, "/bin");

  FILE* ifs = NULL;
  char* prompt = "";
  if (argc == 1)             // configure for interactive mode
  {
    //printf("interactive mode\n"); // for testing only
    prompt = "wish> ";
    ifs = stdin;
  }
  else                       // configure for batch mode
  {
    ifs = fopen(argv[1], "r");
    if (ifs == NULL)
    {
      error();
      exit(1);
    }
    //printf("batch mode\n"); // for testing only
  }  
  
  char *line = NULL;     // buffer for reading in commands
  size_t len = 0;
  // start input processing loop 
  while(1)             
  {
    printf("%s", prompt);                                  // prints prompt in interactive mode, nothing in batch mode
    if (fflush(stdout) == -1)
    {
      perror("");
      exit(1);
    }
    
    if (getline(&line, &len, ifs) != -1)                   // read lines from input stream (prompt or file depending on mode) into buf 
    {
       int count = 0;
       char* token, *delim="&";
       while((token=strsep(&line, delim))!= NULL)          // parse commands separated by '&', and run them in parallel
       {
          run_command(token);
          count++;
       }
        
       for(int i=0;i<count;i++)                            // wait for initiated commands to be completed
         waitpid(-1, NULL, 0);      // not checking success because prints error mulitple times??
       //printf("parent: children processing complete\n"); // for testing purposes 
    }
    else                                                   // if EOF, exit inf loop
    {
      //printf("\n");
      break;
    } 
  } 
  free(line);
  return 0;
}


/*********************************************************************************
Command Handling
**********************************************************************************/
// runs the command
void run_command(char * line)
{
 command_data command;                               
 command = parse_command(line);                      // parse buffer, with name, args and redirection sink
 //print_command(command);                           // for testing purposes
 
 // execute commands, both built-in and generic commands are handled
 if (strcmp(command.command_name, "path") == 0)      // built-in path command 
 {
     //printpath(&search_path);                      // for testing purposes
     if (command.num_args == 0)                      
       clearpath(&search_path);
     else                                            
       set_path(&search_path, command.arglist, command.num_args);
     //printpath(&search_path);                      // for testing purposes
 }
 else if (strcmp(command.command_name, "exit") == 0)  // built-in exit command 
 {
     if (command.num_args > 0)                        
       error();
     else
       exit(0);
 }
 else if (strcmp(command.command_name, "cd") == 0)    // built-in cd command
 {
     if (command.num_args > 1 || command.num_args == 0)
       error();
     else
     {
       if (chdir(command.arglist[0]) == -1)
         error();
     } 
 }
 else                                                // not a built-in command
 {
     //printf("processing generic command\n");         // for testing purposes only
     
     if (command.bad_syntax == 1)
       error(); 
     if (strlen(command.command_name) == 0)          // if command name is empty, ignore 
     {
        //error();
        free_command(&command);
        return;
     }
     //printf("output redirected to: %s\n", command.redir_op);     // for testing purposes only
     
     char* command_path= malloc(sizeof(char)*256);      // variables for storing resolved command path
     find_command(command.command_name, &search_path, command_path);
     if (strlen(command_path) == 0)                    // command_path empty => command not found
       error();
     else                                              
     {
       //printf("command path: %s\n", command_path);     // for testing only
       int rc = fork();
       if (rc < 0)
           error();
       else if (rc == 0)                             // child process to execute the command
       {
           // redirect stdout & stderr to redirection sink 
           if (command.redir_op != NULL && strlen(command.redir_op) != 0)    
           {
             //printf("redir_ip: %s\n", command.redir_op);
             if (close(STDOUT_FILENO) == -1 || close(STDERR_FILENO) == -1)        
             {
               error();
	       exit(0);
	     }
             int fd=-1;
             umask(0);                                    // needed to set the file permission correctly, somehow default is not working 
             if ((fd = open(command.redir_op, O_CREAT|O_WRONLY|O_TRUNC, RWRWRW)) == -1)
             {
                error();
                exit(0);
             }
             if (dup(fd) == -1)
             {
                error();
                exit(0);
             }
             //printf("child:%o\n", prev_mask);
             /***** for testing purposes
             struct stat statbuf;
             stat(command.redir_op, &statbuf);
             printf("child: %o\n", statbuf.st_mode);
             ***************************/
           }              
             
                     
           // build argument list
           char** arg_list = malloc(sizeof(char*)*(command.num_args+2));
           if (arg_list == NULL)
           {
             error();
             exit(1);
           }
           arg_list[0] = command_path;
           for(int i=1; i<=command.num_args; i++)
              arg_list[i] = command.arglist[i-1];
           arg_list[command.num_args+1] = NULL;
           // invoke command
           execvp(command_path, arg_list);
           // only executed if exec call failed
	   error(); 
           free(arg_list); 
           exit(0);              // important to return otherwise will starts its processing loop 
       }
       else                      // parent process
       {
             /* 
             do nothing 
             */
       }	   
     }
     free(command_path); 
 }
 free_command(&command);
}

// parses command name, arglist and redirection sink from string, returns answer in command_data object
command_data parse_command(char* line)
{
  command_data command; 
  command.num_args = 0;
  command.command_name = ""; 
  command.redir_op = "";
  command.bad_syntax = 0;
  // first parse string into section before '>' and after '>' 
  char* delim = ">";
  char* token = NULL, *before_io = NULL, *after_io=NULL;
  int len = 0;
  int output_redirected = 0;
  
  if (strchr(line, '>') != NULL)
   output_redirected = 1; 
  before_io = strsep(&line, delim);   // should be of the form command(not >) [arg1] [arg2].....
 
  if (line != NULL)
  { 
    after_io = malloc(sizeof(char)*(strlen(line)+1));
    strcpy(after_io, line); 
  } 
  
  if (after_io != NULL && strchr(after_io, '>') != NULL)           // presence of one or more '>' after '>' => bad syntax 
  {
    free(after_io);
    command.bad_syntax = 1; 
    return command;
  }     

  delim = " \t\n";
  if (after_io != NULL)                                            // assume everything after '>' is valid, read in information
  {
    while((token=strsep(&after_io, delim))!= NULL)                 
    {
     len = strlen(token);
     if (len == 0)
       continue;
     else
     { 
       command.redir_op = malloc(sizeof(char)*(len+1));            // set destination sink to first word string after '>'
       strcpy(command.redir_op, token);
       break;
     }
    }
  }
 
  
  if (output_redirected == 1 && strlen(command.redir_op) == 0)      // only white spaces after '>' => bad syntax
  {
    command.bad_syntax = 1;
    return command; 
  } 
  while ((token=strsep(&after_io, delim)) != NULL)                 // >1 arg after '>' in after_io => bad syntax 
  {
    if (strlen(token) > 0)
    {
      free(command.redir_op);
      command.bad_syntax = 1;
      return command;
    }
  }
 

  // parse before_io to form command and its arguments 
  delim = " \t\n";
  while((token = strsep(&before_io, delim)) != NULL)         // parse the line and populate command data structure
  {
    len = strlen(token);
    if (len == 0)                                            // continue, if read token was a white space
      continue; 
    if (strchr(delim, token[0]) == NULL)                     
    {
      if (command.num_args == 0)			     // populate command name
      {
	command.command_name = malloc(sizeof(char)*len+1);
        if (command.command_name == NULL)
        {
          error();
          exit(1);
        }
	strcpy(command.command_name, token);
      }
      else                                                   // populate arguments
      { 
	if (command.num_args == 1)
        {
	  command.arglist = malloc(sizeof(char*));
           if (command.arglist == NULL)
           {
             error();
             exit(1);
           }
        }
	else
        {
	  command.arglist = realloc(command.arglist, sizeof(char*)*(command.num_args));
          if (command.arglist == NULL)
          {
            error();
            exit(1);
          }
        }

	command.arglist[command.num_args-1] = malloc(sizeof(char)*len+1);
        if (command.arglist[command.num_args-1] == NULL)
        {
          error();
          exit(1);
        }
	strcpy(command.arglist[command.num_args-1], token);       
      }
      command.num_args++;
    }
  }
  command.num_args -= 1;
  if (strlen(command.command_name) == 0 && output_redirected == 1)
    command.bad_syntax = 1;
  return command;
} 

// frees heap memory associated with the command object
void free_command(command_data* cmd)
{
 if (cmd->command_name != NULL && strlen(cmd->command_name) != 0)
   free(cmd->command_name);
 for(int i=0; i<cmd->num_args; i++)
   free(cmd->arglist[i]);
 if (cmd->redir_op != NULL && strlen(cmd->redir_op) != 0)              // for some reason 2nd check needs to be done, or else segfault
   free(cmd->redir_op);
 cmd->num_args = 0;
}


/**********************************************************************************
Search Path Management
**********************************************************************************/
// clears path list, frees allocated memory on heap
void clearpath(path_list* psearch_path)
{
  if (psearch_path->used == 0)
    return; 
  for(int i=0; i<psearch_path->used; i++)
    free(psearch_path->paths[i]);
  psearch_path->used = 0;
}

// initializes path_list with single path
void initialize_path(path_list* psearch_path, char* path)
{
  psearch_path->used = 1;
  psearch_path->sz = 2;
  psearch_path->paths = malloc(sizeof(char*)*2);
  if (psearch_path->paths == NULL)
  {
    error();
    exit(1);
  }
  psearch_path->paths[0] = malloc(sizeof(char)*(strlen(path)+1));
  if (psearch_path->paths[0] == NULL)
  {
    error();
    exit(1);
  }
  strcpy(psearch_path->paths[0], path);
}

// deletes existing paths, and then populates path list with provided path in char* array
void set_path(path_list* psearch_path, char** paths, int num_paths)
{
  clearpath(psearch_path);
  if (psearch_path->sz < num_paths)
  {
    psearch_path->sz = 2*num_paths;
    psearch_path->paths = realloc(psearch_path->paths, sizeof(char*)*psearch_path->sz);
    if (psearch_path->paths == NULL)
    {
      error();
      exit(1);
    }
  }
  psearch_path->used = num_paths;
  for(int i=0; i<num_paths; i++)
  {
    psearch_path->paths[i] = malloc(sizeof(char)*(strlen(paths[i])+1));
    if (psearch_path->paths[i] == NULL)
    {
      error();
      exit(1);
    }
    strcpy(psearch_path->paths[i], paths[i]);
  }
}

// determines whether command name is available in directories specified in paths_list, if yes, return the full command path, first in the list
void find_command(char* command_name, path_list* psearch_path, char* fullpath)
{ 
  if (psearch_path->used == 0)
    return;
  char *tmppath = malloc(sizeof(char)*256);
  if (tmppath == NULL)
  {
    error();
    exit(1);
  }
  for(int i=0; i<psearch_path->used; i++)
  {
    strcpy(tmppath, psearch_path->paths[i]);
    tmppath = strcat(tmppath, "/");
    tmppath = strcat(tmppath, command_name);
    if (access(tmppath, X_OK) != -1)                // return on the first accessible path found
    {
      strcpy(fullpath, tmppath);
      free(tmppath);
      return;
    }
  }
  fullpath='\0';                                    // necessary for handling in main loop
} 

/**********************************************************************************
Error Handling
**********************************************************************************/
void error()
{
  char error_message[30] = "An error has occurred\n";
  if (write(STDERR_FILENO, error_message, strlen(error_message)) == -1)
  {
    perror("");
    exit(1);
  }
}

/*************************************************
Utilities
*************************************************/
// prints all the paths in path_list, for testing purposes
void printpath(path_list* psearch_path)
{
  if(psearch_path->used == 0)
    return;
  for(int i=0; i<(psearch_path->used-1); i++)
    printf("%s, ", psearch_path->paths[i]);
  printf("%s\n", psearch_path->paths[psearch_path->used-1]);
}

// print the command name, and its argument list, for testing purposes 
void print_command(command_data command)
{
  printf("parsed: %s[%d] ", command.command_name, command.num_args);
  for(int i=0; i<(command.num_args-1); i++)
    printf("%s[%zd], ", command.arglist[i], strlen(command.arglist[i]));
  printf("%s[%zd]\n",command.arglist[command.num_args-1], strlen(command.arglist[command.num_args-1]));
}


