#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

static int nthread = 1;
static int round = 0;

struct barrier {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  int nthread;      // Number of threads that have reached this round of the barrier
  int round;     // Barrier round
} bstate;

static void
barrier_init(void)
{
  assert(pthread_mutex_init(&bstate.barrier_mutex, NULL) == 0);
  assert(pthread_cond_init(&bstate.barrier_cond, NULL) == 0);
  bstate.nthread = 0;
}

static void 
barrier( long tn)
{
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.
  //
  //  pthread_cond_wait(&bstate.barrier_cond,&bstate.barrier_mutex);
   pthread_mutex_lock(&bstate.barrier_mutex);
    // printf("tn=%ld lock one\n",tn);
   bstate.nthread++;

  int round_before = bstate.round; 
   if(bstate.nthread == nthread){
     pthread_cond_broadcast(&bstate.barrier_cond);
    //  printf("tn=%ld round=%d\n",tn, bstate.round);
     bstate.round++;
     bstate.nthread=0;

   }else{


    //  while(round_before != (bstate.round-1)){
      // printf("tn=%ld sleep round=%d now round=%d\n",tn,round_before,bstate.round);
      pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
      // if(round_before == bstate.round)
      // printf("wakeup tn=%ld before round=%d now round=%d\n", tn, round_before, bstate.round);
    //  }
      // printf("before2 round=%d now round=%d\n ",round_before,bstate.round);
   }
  
  //  bstate.nthread--;
  //  pthread_cond_broadcast(&bstate.barrier_cond);
   pthread_mutex_unlock(&bstate.barrier_mutex);
    // printf("tn=%ld lock end\n\n",tn);
  // usleep(2000);
}

static void *
thread(void *xa)
{
  long n = (long) xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++) {
    int t = bstate.round;
    // printf("thread_num=%ld i=%d \n",n,i);
    if(i!=t)
     printf("thread_num=%ld i=%d t=%d \n",n,i,t);
    assert (i == t);
    barrier(n);
    usleep(random() % 10);
  }

  return 0;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  barrier_init();

  for(i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *) i) == 0);
  }
  for(i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
}
