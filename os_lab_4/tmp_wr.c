#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <cstring>
#include <errno.h>

#define SHMSIZE sizeof(char)
#define PERMS (S_IRWXU | S_IRWXG | S_IRWXO)
#define NW  4
#define NR  5

#define C_WAITING_R 0   // readers_queue
#define C_ACTIVE_R 1    // numb_of_readers
#define C_WAITING_W 2   // writers_queue
#define B_ACTIVE_W 3    // active_writer
#define B_SEM 4         // bin_sem

struct sembuf sem_start_read[] = {
    { C_WAITING_R,  1, SEM_UNDO },
    //{ B_ACTIVE_W,   0, SEM_UNDO },
    { B_SEM,	    0, SEM_UNDO },
    { C_WAITING_W,  0, SEM_UNDO },
    { C_ACTIVE_R,   1, SEM_UNDO },
    { C_WAITING_R, -1, SEM_UNDO },
};

struct sembuf sem_stop_read[] = {
    { C_ACTIVE_R,  -1, SEM_UNDO }
};

struct sembuf sem_start_write[] = {
    { C_WAITING_W,  1, SEM_UNDO },
    { C_ACTIVE_R,   0, SEM_UNDO },
    { B_SEM,	    1, SEM_UNDO },
    { B_ACTIVE_W,  -1, SEM_UNDO },
    { C_WAITING_W, -1, SEM_UNDO },
};

struct sembuf sem_stop_write[] = {
    { B_ACTIVE_W,   1, SEM_UNDO },
    { B_SEM,	   -1, SEM_UNDO }
};

int semid;
int fl = 1;

void sig_handler(int sig_num)
{
    fl = 0;
    printf("pid: %d, signal: %d\n", getpid(), sig_num);
}

int start_read(const int semid)
{
    return semop(semid, sem_start_read, 5);
}

int stop_read(const int semid)
{
    return semop(semid, sem_stop_read, 1);
}

int start_write(const int semid)
{
    return semop(semid, sem_start_write, 5);
}

int stop_write(const int semid)
{
    return semop(semid, sem_stop_write, 2);
}

void reader(int semid, char *ch)
{
    srand(time(NULL) + getpid());
    
    while (fl)
    {
        usleep((0.2 + (double)rand() / RAND_MAX) * 1000000);
        if (start_read(semid) == -1) {
        	char err_msg[100];
            sprintf(err_msg, "Error: semop (start_read) pid = %d, errno %d", getpid(), errno);
            perror(err_msg);
            exit(1);
        }
        
        printf("	Reader %5d <<< %s\n", getpid(), ch);
        
        if (stop_read(semid) == -1) {
        	char err_msg[100];
            sprintf(err_msg, "Error: semop (stop_read) pid = %d, errno %d", getpid(), errno);
            perror(err_msg);
            exit(1);
        }
    }
    exit(0);
}

void writer(int semid, char *ch)
{
    srand(time(NULL) + getpid());
    
    while (fl)
    {
        usleep((0.2 + (double)rand() / RAND_MAX) * 1000000);
        if (start_write(semid) == -1) {
        	char err_msg[100];
            sprintf(err_msg, "Error: semop (start_write) pid = %d, errno %d", getpid(), errno);
            perror(err_msg);
            exit(1);
        }
        
        if (*ch == 'z')
        	*ch = 'a'-1;
        (*ch)++;
		printf("Writer %5d >>> %s\n", getpid(), ch);
		
        
        if (stop_write(semid) == -1) {
        	char err_msg[100];
            sprintf(err_msg, "Error: semop (stop_write) pid = %d, errno %d", getpid(), errno);
            perror(err_msg);
            exit(1);
        }
    }
    exit(0);
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
    	perror("Error: signal.\n");
    	exit(EXIT_FAILURE);
    }

    pid_t pids[NW + NR];

	// Преобразование имени файла и идентификатора проекта в ключ для системных вызовов
    key_t shmkey = ftok("keyfile", 1);
    if (shmkey == (key_t) -1) { perror("Error: ftok\n"); exit(1); }
    
	// Создание сегмента разделяемой памяти или подтверждение прав доступа (возвр. - идентефикатор)
    int shmid = shmget(shmkey, SHMSIZE, IPC_CREAT | PERMS);
    if (shmid == -1) { perror("Error: shmget\n"); exit(1); }

	// Присоединение сегмента разделяемой памяти к адресному пр-ву процесса
    char *shmaddr = (char*)shmat(shmid, 0, 0);
    if (shmaddr == (char*)-1) { perror("Error: shmat\n"); exit(1); }
    
    *shmaddr = 'a'-1;

    int semkey = ftok("keyfile", 2);
    if (semkey == (key_t) -1) { perror("Error: ftok\n"); exit(1); }
    
    // Создание нового набора семафоров или открытие уже имеющегося
	if ((semid = semget(semkey, 5, IPC_CREAT | PERMS)) == -1) { perror("Error: semget\n"); exit(1); }

	// Изменение управляющих параметров набора семафоров
    int cbsaw = semctl(semid, B_ACTIVE_W, SETVAL, 1);
	if (cbsaw == -1) { perror("Error: semctl\n"); exit(1); }
	cbsaw = semctl(semid, B_SEM, SETVAL, 0);
	if (cbsaw == -1) { perror("Error: semctl\n"); exit(1); }
	cbsaw = semctl(semid, C_WAITING_R, SETVAL, 0);
	if (cbsaw == -1) { perror("Error: semctl\n"); exit(1); }
	cbsaw = semctl(semid, C_ACTIVE_R, SETVAL, 0);
	if (cbsaw == -1) { perror("Error: semctl\n"); exit(1); }
	cbsaw = semctl(semid, C_WAITING_W, SETVAL, 0);
	if (cbsaw == -1) { perror("Error: semctl\n"); exit(1); }
	

    for (int i = 0; i < NW; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1) { perror("Error: w can't fork\n"); exit(1); }
        if (pids[i] == 0) { writer(semid, shmaddr); }
    }
    for (int i = 0; i < NR; i++)
    {
        pids[NW + i] = fork();
        if (pids[NW + i] == -1) { perror("Error: r can't fork\n"); exit(1); }
        if (pids[NW + i] == 0) { reader(semid, shmaddr); }
    }

	int status;

	for (int i = 0; i < (NW + NR); i++) 
    {
        pid_t w = waitpid(pids[i], &status, 0);
        if (w == -1) {
            perror("Error: waitpid.\n");
            exit(EXIT_FAILURE);
        }
		
		if (WIFEXITED(status)) {
		   printf("exited, status=%d, PID = %d\n", WEXITSTATUS(status), w);
	   	} else if (WIFSIGNALED(status)) {
		   printf("killed by signal %d, PID = %d\n", WTERMSIG(status), w);
	   	} else if (WIFSTOPPED(status)) {
		   printf("stopped by signal %d, PID = %d\n", WSTOPSIG(status), w);
	   	}
    }

	
    // Отсоединение сегмента разделяемой памяти от адресного пр-ва процесса
    if (shmdt(shmaddr) == -1) { perror("Error: shmdt\n"); exit(1); }

	// Удаление сегмента разделяемой памяти
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: rm shm error\n");
        exit(1);
    }

	// Удаление набора семафоров
	if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Error: rm sem error\n");
        exit(1);
    }
    
}
