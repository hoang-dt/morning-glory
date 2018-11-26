#include <pthread.h>
#include <stdio.h>

/* this function is run by the second thread */
void *inc_x(void *x_void_ptr)
{

/* increment x to 100 */
int *x_ptr = (int *)x_void_ptr;
while(++(*x_ptr) < 200000000);

printf("x increment finished\n");

/* the function must return something - NULL will do */
return NULL;

}

int main()
{

int x = 0, y = 0;

/* show the initial values of x and y */
printf("x: %d, y: %d\n", x, y);

/* this variable is our reference to the second thread */
pthread_t inc_x_thread1;
pthread_t inc_x_thread2;
pthread_t inc_x_thread3;
pthread_t inc_x_thread4;
pthread_t inc_x_thread5;
pthread_t inc_x_thread6;
pthread_t inc_x_thread7;
pthread_t inc_x_thread8;

/* create a second thread which executes inc_x(&x) */
if(pthread_create(&inc_x_thread1, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread2, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread3, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread4, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread5, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread6, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread7, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
if(pthread_create(&inc_x_thread8, NULL, inc_x, &x)) {

fprintf(stderr, "Error creating thread\n");
return 1;

}
/* increment y to 100 in the first thread */
while(++y < 100);

printf("y increment finished\n");

/* wait for the second thread to finish */
if(pthread_join(inc_x_thread1, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}

if(pthread_join(inc_x_thread2, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread3, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread4, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread5, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread6, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread7, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
if(pthread_join(inc_x_thread8, NULL)) {

fprintf(stderr, "Error joining thread\n");
return 2;

}
/* show the results - x is now 100 thanks to the second thread */
printf("x: %d, y: %d\n", x, y);

return 0;

}
