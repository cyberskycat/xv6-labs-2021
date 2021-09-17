#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    if( argc < 2 ){
    fprintf(2, "Usage: sleep 1 (sec)...\n");
    exit(1);
  }
  int sleepsec = atoi(argv[1]);
  sleep(sleepsec);  
  exit(0);
}
