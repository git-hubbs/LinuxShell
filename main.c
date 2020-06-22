/****************************************************
Author: Jason Hubbs                                 *
Date: 06-22-20                                      *
Descripton: Linux shell that handles an arbitrary   *
            number of pipes (up to 50). Program     *
            exits apon the command 'exit'.          *
****************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

//Declare program functions and max variables
void Loop();
int PopulateCommand(char *command[10][10], int num_words, char **line_words);
int RunInitialPipeCommands(int pipeIdx, int pfds[pipeIdx][2], char *command[10][10]);
int RunFinalPipeCommand(int pipeIdx, int pfds[pipeIdx][2], char *command[10][10]);
int ParseLine(char* line, char** list_to_populate);
const int MAX_LINE_WORDS = 100, MAX_LINE_CHARS = 1000;

//Driver function starts main program loop.
int main(){
  Loop();
  return 0;
}

//Loop accepts input commands and runs them.
void Loop(){
  char line[MAX_LINE_CHARS];
  while( fgets(line, MAX_LINE_CHARS, stdin) ) {
    char *line_words[MAX_LINE_WORDS + 1];
    int num_words = ParseLine(line, line_words);
    char *command[10][10] = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};
    int cmdIdx = 0;
    int pipeIdx = PopulateCommand(command, num_words, line_words);
    int pfds[pipeIdx][2];
    if(pipeIdx > 0){
      RunInitialPipeCommands(pipeIdx, pfds, command);
      RunFinalPipeCommand(pipeIdx, pfds, command);
    }else
      RunFinalPipeCommand(pipeIdx, pfds, command);
    wait(NULL);
  }
}

//Populates 2d array storing command parsed by pipes and words
int PopulateCommand(char *command[10][10], int num_words, char **line_words){
  int cmdIdx = 0, pipeIdx = 0;
  for(int i = 0; i < num_words; i++){
    if(strcmp(line_words[i], "|")){
      command[pipeIdx][cmdIdx] = line_words[i];
      cmdIdx++;
    }else{
      cmdIdx = 0;
      pipeIdx++; 
    }
  }return pipeIdx;
}

//Runs first n-1 rows in command array and pipes their result forward.
int RunInitialPipeCommands(int pipeIdx, int pfds[pipeIdx][2], char *command[10][10]){
  for (int i=0; i < pipeIdx; i++) {
    pipe(pfds[i]);
    int pid = fork();
    if (pid == 0) {
      if (i > 0) {
	dup2(pfds[i-1][0], 0);
	close(pfds[i-1][1]);
	close(pfds[i-1][0]);
      }
      dup2(pfds[i][1], 1);
      close(pfds[i][1]);
      close(pfds[i][0]);
      if(execvp(command[i][0], command[i]))
	printf("Error running command.\n");
      kill(getpid(), 8);
    }
  }return 0;
}

//Runs n row in command array and outputs result to STDOUT
int RunFinalPipeCommand(int pipeIdx, int pfds[pipeIdx][2], char *command[10][10]){
  int pid = fork();
  if (pid == 0) {
    if (pipeIdx > 0) {
      dup2(pfds[pipeIdx-1][0], 0);
      close(pfds[pipeIdx-1][1]);
      close(pfds[pipeIdx-1][0]);
    }if(execvp(command[pipeIdx][0], command[pipeIdx]))
       printf("Error running command.\n");
    kill(getpid(), 8);
  }
}

//Parses command line into words by spaces
int ParseLine(char* line, char** list_to_populate) {
  char* saveptr;
  char* delimiters = " \t\n";
  int i = 0;
  list_to_populate[0] = strtok_r(line, delimiters, &saveptr);
  while(list_to_populate[i] != NULL && i < MAX_LINE_WORDS - 1){
    list_to_populate[++i] = strtok_r(NULL, delimiters, &saveptr);
  }
  return i;
}
