#include <stdio.h>
#include <time.h>
#include <windows.h>

#define MAX_TIME_SLEEP 1000 // мс
#define READERS_NUM 5
#define WRITERS_NUM 6
HANDLE mutex;
HANDLE can_read;
HANDLE can_write;
LONG writers_queue = 0;
LONG readers_queue = 0;
LONG active_readers = 0;
LONG active_writer = 0;
char buf = 'a';
int f_sigint = 1;

BOOL WINAPI CtrlCHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("Ctrl-C event\n");
        f_sigint = 0;
        return TRUE;
    }
    return FALSE;
}

void start_read() {
    InterlockedIncrement(&readers_queue);
    if (active_writer || writers_queue)
        WaitForSingleObject(can_read, INFINITE);
    WaitForSingleObject(mutex, INFINITE);
    InterlockedDecrement(&readers_queue);
    InterlockedIncrement(&active_readers);
    SetEvent(can_read);
    ReleaseMutex(mutex);
}

void stop_read() {
    InterlockedDecrement(&active_readers);
    if (active_readers == 0)
        SetEvent(can_write);
}

void start_write() {
    InterlockedIncrement(&writers_queue);
    if (active_readers || active_writer)
        WaitForSingleObject(can_write, INFINITE); 
    InterlockedDecrement(&writers_queue);
    InterlockedIncrement(&active_writer);
}

void stop_write() {
    InterlockedDecrement(&active_writer);
    ResetEvent(can_write);
    if (readers_queue > 0)
        SetEvent(can_read);
    else
        SetEvent(can_write);
}

DWORD WINAPI reader(PVOID ptr) {	
    int id = *(int *)ptr;
	srand(time(NULL));
    while (f_sigint) {
        Sleep(rand() % MAX_TIME_SLEEP + 1000);
        start_read();
        printf("Reader %d read: %c\n", id + 1, buf);
        stop_read();
    }
    return 0;
}

DWORD WINAPI writer(PVOID ptr) {
    int id = *(int *)ptr;
	srand(time(NULL));
    while (f_sigint) {
        Sleep(rand() % MAX_TIME_SLEEP + 1000);
        start_write();
        if (buf == 'z')
            buf = 'a';
        else
            buf++;
        printf("Writer %d wrote: %c\n", id + 1, buf);
        stop_write();
    }
    return 0;
}

int main(void)
{
    HANDLE reader_threads[WRITERS_NUM], writer_threads[READERS_NUM];
	int readers_id[WRITERS_NUM], writers_id[READERS_NUM];
    DWORD id = 0;
    if (SetConsoleCtrlHandler(CtrlCHandler, TRUE) == 0) {
        perror("SetConsoleCtrlHandler");
        exit(1);
    }
    if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
        perror("CreateMutex");
        exit(1);
    }
    if ((can_read = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) { 
        perror("CreateEvent");
        exit(1);
    }
    if ((can_write = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        perror("CreateEvent");
        exit(1);
    }
    for (int i = 0; i < READERS_NUM; i++) {
        readers_id[i] = i;
        if ((reader_threads[i] = CreateThread(NULL, 0, reader, readers_id + i, 0, &id)) == NULL) {
            perror("CreateThread");
            exit(1);
        }
    }
    for (int i = 0; i < WRITERS_NUM; i++) {
        writers_id[i] = i;
        if ((writer_threads[i] = CreateThread(NULL, 0, writer, writers_id + i, 0, &id)) == NULL) {
            perror("CreateThread");
            exit(1);
        }
    }
	WaitForMultipleObjects(READERS_NUM, reader_threads, TRUE, INFINITE);
	WaitForMultipleObjects(WRITERS_NUM, writer_threads, TRUE, INFINITE);
    for (int i = 0; i < READERS_NUM; i++)
        CloseHandle(reader_threads[i]);
    for (int i = 0; i < WRITERS_NUM; i++)
        CloseHandle(writer_threads[i]);
    CloseHandle(can_read);
    CloseHandle(can_write);
    CloseHandle(mutex);
	return 0;
}
