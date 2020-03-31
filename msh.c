/*

  Name: Bhupendra Ramdam
  ID:   10013370027

 */

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments


void history(char *token[], char *history[], int count_his, char *cmd_str);
void parse_semicolon(char *cmd_str);

//handle signal send by user
static void handle_signal(int sig)
{

}

int main()
{
    struct sigaction act;

    /*
      Zero out the sigaction struct
     */
    memset(&act, '\0', sizeof (act));

    /*
      Set the handler to use the function handle_signal()
     */
    act.sa_handler = &handle_signal;

    /*
      Install the handler for Ctrl-c and check the return value.
     */
    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        perror("sigaction: ");
        return 1;
    }
    /*
      Install the handler for Ctrl-z and check the return value.
     */

    if (sigaction(SIGTSTP, &act, NULL) < 0)
    {
        perror("sigaction: ");
        return 1;
    }

    //storing the commands entered by user
    char * cmd_str = (char*) malloc(MAX_COMMAND_SIZE);

    char *histo[15];
    int pid_history[15];
    int pid_index=0;
    int echocheck=0; //echocheck is false when the ! has not been called to redo the task

    int i = 0, count_his = 0;
    for (i = 0; i < 15; i++)
    {
        histo[i] = malloc(MAX_COMMAND_SIZE);//allocating space dynamically on histo
    }


    while (1)
    {
        // Print out the msh prompt
        printf("msh> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;

        char *working_str = strdup(cmd_str);

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input stringswith whitespace used as the delimiter
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
                (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }
        histo[count_his++] = strdup(cmd_str);


        //if user entered enter then,it continues to execute
        if (cmd_str[0] == '\n')
        {
            continue;
        }
        // this will handle blank line(space) if user input ' ' and enter
        if (cmd_str[0] == ' ')
        {
            continue;
        }


        //if the user input is exit or quit should terminate the mav shell with status zero
        if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
        {
            exit(0);
        }

        //when user type command with "!", this part will go and
        //print the listed command of particular number
        if (token[0][0] == '!')
        {
            history(token, histo, count_his, cmd_str);
            echocheck=1;//avoid echo part when there is !
        }
        //this will prints out history when typed history
        //strdup will duplicate the cmd_str and store in histo
        //cout_his stores the number of command entered but should not exceed 15
        if (strcmp(token[0], "history") == 0)
        {
            if (count_his < 15)
            {
                for (i = 0; i < count_his; i++)
                {
                    printf("%d: %s", i, histo[i]);
                }
            }
						//will list out first 15 command if exceeds more than 15 commands
            else if (count_his > 14)
            {

                printf("command exceeds more than 15.\n");
                for (i = 0; i < 15; i++)
                {
                    printf("%d: %s", i, histo[i]);
                }

            }

            continue;
        }
        //command typed in the format echo foo ; echo bar ; echo baz
        //if command start with echo and echocheck is zero
        //when echocheck is zero, then will call function parse_semicolon
        //which handles the echo commands and perform the task
        if ((strcmp(token[0], "echo") == 0)&&(echocheck==0))
        {

            parse_semicolon(cmd_str);
            continue;
        }
        //but when echocheck is 1 then it will jump program to the begining of while loop
        //as the ! command has been requested which has already taken care of the
        // echo command with in itself
        if(echocheck==1&&(strcmp(token[0], "echo") == 0))
          continue;
        //when we entered bg then we will send SIGCONT to the last susspended part and the
        //process will resume on the background
        if(strcmp(token[0],"bg")==0)
        {

          if(pid_index!=-1)//handle negative array index
          {
            kill(pid_history[pid_index-1],SIGCONT);
          }
          continue;


        }

        //this displays the pids of parent process when typed listpids
        //more than 15 pids are not printed
        if (strcmp(token[0], "listpids") == 0)
        {
            if (pid_index < 15)
            {
                for (i = 0; i < pid_index; i++)
                {
                    printf("%d:%d\n", i, pid_history[i]);
                }
            }
            else
            {
                printf("Out of scope. Cannot print PIDs.\n");
            }
            continue;

        }

        //chdir changes the  current working directory
        if (strcmp(token[0], "cd") == 0)
        {
            chdir(token[1]);

        }
        else
        {
            //fork the process and
            //saving all the pid_no that we get by forking in an array pid_history
            pid_t pid_no = fork();
            pid_history[pid_index++] = pid_no;
            int status;

            if (pid_no == 0)// if returns 0, we are in child process
            {
                //execvp provides an array of pointer to null terminated string
                if (execvp(token[0], token) == -1)
                {
                    printf("%s: Command not found.\n", token[0]);
                    exit(EXIT_FAILURE);
                }
                execvp(token[0], token);
                exit(EXIT_SUCCESS);
            }
            //Force the parent process to wait until the child process

                waitpid(pid_no, &status, 0);

        }
        free(working_root);
        echocheck=0;//reseting the value of echocheck
    }
    return 0;
}

//when ! is typed ,this function rewrite the token
//this function will pass the argument token , argument history containing all the history,
//argument count_his that will keep track of number of history
//argument cmd_str stores the copied history

void history(char *token[], char *history[], int count_his, char *cmd_str)
{
    //pointer to the token should be dereference so that we can change
    //string into integer

    int n = atoi(&token[0][1]);

    //the input number greater than no of commands and less than 0
    //will give the prompt
    if ((n) >= count_his || (n) < 0)
    {
        printf("Command not in history\n");

    }
    //otherwise work on the command related to that number. cmd_str get copied and
    //we are repeating the process
    else
    {
        cmd_str = strdup(history[n]);

        int token_count = 0;
        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;
        char *working_str = strdup(cmd_str);
        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;


        // Tokenize the input stringswith whitespace used as the delimiter
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
                (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        free(working_root);

    }
    //this portion will handle echo when user call the echo part with !
    if (strcmp(token[0], "echo") == 0)
    {
        parse_semicolon(cmd_str);
    }
}
// This function passes the cmd_str entered buy user as argument.
//and parse the user command with echo separated by ';'and ' ' and
// prints out the parameters after echo
void parse_semicolon(char *cmd)
{
    int count = 0;
    int i;

    char echostring[3][50]; //initialize the 2-d array to store on it
    char* token = strtok(cmd, "; "); //tokenizing with "; " will separate first echo
    while (token != NULL)
    {
        if (strcmp(token, "echo") != 0)
        {
            //those besides echo from command will be copied to echostring
            strcpy(echostring[count], token);
            count++;
        }
        token = strtok(NULL, "; "); //tokenizing the string from null to next ";" or " "

    }
    for (i = 0; i < count; i++)
    {
        int status;
        pid_t pid = fork();
        if (pid == 0)
        {
            //echostring is the list of argument that we get after tokenizing
            execl("/bin/echo","echo", echostring[i], NULL);
            exit(EXIT_SUCCESS);
        }
        waitpid(pid, &status, 0);
    }

}
