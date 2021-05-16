#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password 
     on the next.
     DO NOT add any prompts.  The only output of this program will be one 
	 of the messages defined above.
     Please read the comments in validate carefully.
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  
  int pip_fd[2];
  if(pipe(pip_fd) == -1){
    perror("pipe");
    exit(1);
  }
  char *temp;
  if((temp = strchr(user_id, '\n')) == NULL) {
    user_id[MAXLINE - 1] = '\0';
  }
  if((temp = strchr(password, '\n')) == NULL) {
    password[MAXLINE - 1] = '\0';
  }
  int result = fork();
  if(result<0){
    perror("fork");
    exit(1);
  }
  else if(result == 0){
    if(dup2(pip_fd[0], fileno(stdin)) == -1){
      perror("dup2");
      exit(1);
    }
    if(close(pip_fd[0]) == -1){
      perror("close");
      exit(1);
    }
    if(close(pip_fd[1]) == -1){
      perror("close");
      exit(1);
    }
    execl("./validate", "validate", NULL);
  }
  else{
    int status;
    if(close(pip_fd[0]) == -1){
      perror("close");
      exit(1);
    }
    if(write(pip_fd[1], user_id, 10) == -1){
      perror("write");
      exit(1);
    }
    if(write(pip_fd[1], password, 10) == -1){
      perror("write");
      exit(1);
    }
    wait(&status);
    if(WIFEXITED(status)){
      if(WEXITSTATUS(status) == 0){
        printf(SUCCESS);
      }
      if(WEXITSTATUS(status) == 1){
        printf("error occur");
      }
      if(WEXITSTATUS(status) == 2){
        printf(INVALID);
      }
      if(WEXITSTATUS(status) == 3){
        printf(NO_USER);
      }
    }
  }
  return 0;
}
