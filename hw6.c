#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>

#define TIME_TO_RUN 30
#define BUFFER_SIZE 512
#define READ_END 0
#define WRITE_END 1

int main()
{
	int i = 0, count = 1;
	pid_t pid;
	int sleep_time, status, retval, rbytes, maxfd, file_out;
	fd_set rfds;
	struct timeval tv, begin, end, current;
	char readbuffer[BUFFER_SIZE];
	double time_spent = 0;
	double current_time;
	char output[BUFFER_SIZE];
	
	srand(time(NULL));
	
	int pipes[10];
/*
	[0][1] [read][write]
	[2][3] [read][write]
	[4][5] [read][write]
	[6][7] [read][write]
	[8][9] [read][write]
*/
	
	
	for(i=0; i<5; i++)
		pipe(pipes + 2*i);
		
	char *pos;
		
	gettimeofday(&begin, NULL);
	int process_ID[5];

	if((pid = fork()) == 0) //FIRST CHILD [0][1]
	{				
		process_ID[0] = pid;	
		close(pipes[READ_END]);
		while(time_spent < TIME_TO_RUN)
		{
			gettimeofday(&current, NULL);
			current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
						   (double) (current.tv_sec - begin.tv_sec);

			sprintf(output, "%.3f: Child 1: Message #%d", current_time, count++);

			write(pipes[WRITE_END], output, sizeof(output));
			gettimeofday(&end, NULL);
			time_spent = (double)(end.tv_sec - begin.tv_sec);
			sleep(rand() % 3);
		}
	} 
	else 
	{
		process_ID[1] = pid;
		if((pid = fork())==0) //SECOND CHILD // second child goes in this loop //second child	[2][3]
		{
			close(pipes[READ_END + 2]);				
			while(time_spent < TIME_TO_RUN)
			{
				gettimeofday(&current, NULL);
				current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
						   	(double) (current.tv_sec - begin.tv_sec);
				sprintf(output, "%.3f: Child 2: Message #%d", current_time, count++);
				write(pipes[WRITE_END + 2], output, sizeof(output));
				gettimeofday(&end, NULL);
				time_spent = (double)(end.tv_sec - begin.tv_sec);
				sleep(rand() % 3);
			}
			
			exit(0);
		} 
		else 
		{
			if((pid = fork())==0)
			{			//third child	[4][5]
				process_ID[2] = pid;	
				close(pipes[READ_END + 4]);
				while(time_spent < TIME_TO_RUN)
				{
					gettimeofday(&current, NULL);
					current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
						   (double) (current.tv_sec - begin.tv_sec);

					sprintf(output, "%.3f: Child 3: Message #%d", current_time, count++);

					write(pipes[WRITE_END + 4], output, sizeof(output));
					gettimeofday(&end, NULL);
					time_spent = (double)(end.tv_sec - begin.tv_sec);
					sleep(rand() % 3);
				}
				exit(0);
			}
			 else
			 {
				
				if((pid = fork())==0)		//fourth child	[6][7]
				{
					process_ID[3] = pid;
					close(pipes[READ_END + 6]);
					while(time_spent < TIME_TO_RUN)
					{
						gettimeofday(&current, NULL);
						current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
						   	(double) (current.tv_sec - begin.tv_sec);
						sprintf(output, "%.3f: Child 4: Message #%d", current_time, count++);
						write(pipes[WRITE_END + 6], output, sizeof(output));
						gettimeofday(&end, NULL);
						time_spent = (double)(end.tv_sec - begin.tv_sec);
						sleep(rand() % 3);
					}
					
					exit(0);
				} 
				else
				{
					if((pid = fork())==0) // FIFTH CHILD //fifth child	[8][9]
					{
						process_ID[4] = pid;	
						char buf[BUFFER_SIZE];
						while(time_spent < TIME_TO_RUN)
						{
							fgets(buf, BUFFER_SIZE, stdin);
							gettimeofday(&current, NULL);
							current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
										   (double) (current.tv_sec - begin.tv_sec);
							sprintf(output, "%.3f: Child 5: Message %s", current_time, buf);
							if ((pos=strchr(output, '\n')) != NULL)
								*pos = '\0';
							write(pipes[WRITE_END + 8], output, strlen(output));
							gettimeofday(&end, NULL);
							time_spent = (double)(end.tv_sec - begin.tv_sec);
						}
						exit(0);
					} 
					else //PARENT 
					{	

						file_out = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0777);
						
						do{
							maxfd = 0;
							FD_ZERO(&rfds);
							for(i=0; i<5; i++)
								FD_SET(pipes[i*2], &rfds);
							tv.tv_sec = 2;
							tv.tv_usec = 0;		
							for(i=0; i<5; i++)
								maxfd = (maxfd > pipes[i*2])? maxfd : pipes[i*2];
							retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
							if(retval == -1)
								printf("Select failed.\n");
							else if(retval){
								for(i=0; i<5; i++){									
									if(FD_ISSET(pipes[i*2], &rfds)){
										gettimeofday(&current, NULL);
										current_time = (double) (current.tv_usec - begin.tv_usec) / 1000000 + 
													   (double) (current.tv_sec - begin.tv_sec);
										memset(readbuffer,0,strlen(readbuffer));
										rbytes = read(pipes[i*2], readbuffer, BUFFER_SIZE);
										if(rbytes > 0)
											sprintf(output, "%s has arrived at %.3f\n", readbuffer, current_time);
											write(file_out, output, strlen(output));
										break;
									}
								}
							}
							else{}
								
							gettimeofday(&end, NULL);
							time_spent = (double)(end.tv_sec-begin.tv_sec);
						}while(time_spent < TIME_TO_RUN);
						close(file_out);
					}					
				}				
			}		
		}	
	}
	
	for (i = 0; i < 5; i++)
		kill(process_ID[i]);
	return 0;
}
