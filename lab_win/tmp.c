#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define READERS_CNT 5
#define WRITERS_CNT 3

static HANDLE canRead;
static HANDLE canWrite;
static HANDLE mutex;

static LONG activeReaders = 0;
static LONG waitingReaders = 0;
static LONG waitingWriters = 0;
static LONG activeWriter = 0; // 0 = нет писателя, 1 = есть

// Глобальный флаг выполнени
static volatile LONG fl = 1;

static char value = 'a';

BOOL WINAPI ConsoleHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_C_EVENT) {
        // Пользователь нажал Ctrl+C
        InterlockedExchange(&fl, 0);
        // Не вызываем ExitProcess, просто возвращаем TRUE, чтобы сообщить что обработали.
        return TRUE;
    }
    return FALSE; // Для других событий пусть работает дефолтная обработка
}

void start_read() {
    InterlockedIncrement(&waitingReaders);

    // Читатель ждет, если есть активный писатель или ожидающие писатели.
    if (activeWriter == 1  waitingWriters > 0)
        WaitForSingleObject(canRead, INFINITE);

    WaitForSingleObject(mutex, INFINITE);
    InterlockedDecrement(&waitingReaders);
    InterlockedIncrement(&activeReaders);
    // Цепная реакция: разбудим следующего читателя, если он есть
    SetEvent(canRead);
    ReleaseMutex(mutex);
}

void stop_read() {
    InterlockedDecrement(&activeReaders);

    // Если нет активных читателей, сигнализируем писателям, что они могут писать
    if (InterlockedCompareExchange(&activeReaders, 0, 0) == 0)
        SetEvent(canWrite);
}

void start_write() {
    InterlockedIncrement(&waitingWriters);

    // Писатель ждет, если есть активные читатели или активный писатель
    if (activeReaders > 0  activeWriter == 1)
        WaitForSingleObject(canWrite, INFINITE);

    InterlockedDecrement(&waitingWriters);
    InterlockedExchange(&activeWriter, 1);
}

void stop_write() {
    InterlockedExchange(&activeWriter, 0);

    // Если есть ожидающие читатели, даем им приоритет:
    if (InterlockedCompareExchange(&waitingReaders, 0, 0) > 0)
        SetEvent(canRead);
    else
        SetEvent(canWrite);
}

DWORD WINAPI ReaderThread(LPVOID param) {
    int id = (int)(intptr_t)param;
    srand(GetCurrentThreadId());

    while (InterlockedCompareExchange(&fl, 0, 0)) {
        Sleep(rand() % 2000 + 1000);

        start_read();
        printf("Reader %d read: %c\n", id, value);
        stop_read();
    }

    return 0;
}

DWORD WINAPI WriterThread(LPVOID param) {
    int id = (int)(intptr_t)param;
    srand(GetCurrentThreadId());

    while (InterlockedCompareExchange(&fl, 0, 0)) {
        Sleep(rand() % 2000 + 1000);

        start_write();
        value++;
        if (value > 'z')
            value = 'a';
        printf("Writer %d wrote: %c\n", id, value);
        stop_write();
    }

    return 0;
}

int main(void) {
    srand((unsigned int)time(NULL));

    // Установим обработчик Ctrl+C
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    mutex = CreateMutex(NULL, FALSE, NULL);
    if (!mutex) {
        fprintf(stderr, "Failed to create mutex\n");
        return 1;
    }

    canRead = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!canRead) {
        fprintf(stderr, "Failed to create canRead event\n");
        return 1;
    }

    canWrite = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!canWrite) {
        fprintf(stderr, "Failed to create canWrite event\n");
        return 1;
    }

    HANDLE readers[READERS_CNT];
    DWORD readerIDs[READERS_CNT];

    HANDLE writers[WRITERS_CNT];
    DWORD writerIDs[WRITERS_CNT];

    // Создаем потоки читателей
    for (int i = 0; i < READERS_CNT; i++) {
        readers[i] = CreateThread(NULL, 0, ReaderThread, (LPVOID)(intptr_t)i, 0, &readerIDs[i]);
        if (!readers[i]) {
            fprintf(stderr, "Failed to create reader thread\n");
            return 1;
        }
    }
    
// Создаем потоки писателей
    for (int i = 0; i < WRITERS_CNT; i++) {
        writers[i] = CreateThread(NULL, 0, WriterThread, (LPVOID)(intptr_t)i, 0, &writerIDs[i]);
        if (!writers[i]) {
            fprintf(stderr, "Failed to create writer thread\n");
            return 1;
        }
    }

    // Ожидаем завершения читателей по одному
    for (int i = 0; i < READERS_CNT; i++) {
        DWORD dw = WaitForSingleObject(readers[i], INFINITE);
        switch (dw) {
        case WAIT_OBJECT_0:
            printf("reader thread %d finished\n", readerIDs[i]);
            break;
        case WAIT_TIMEOUT:
            printf("waitThread timeout %lu\n", dw);
            break;
        case WAIT_FAILED:
            printf("waitThread failed %lu\n", dw);
            break;
        default:
            printf("unknown %lu\n", dw);
            break;
        }
        CloseHandle(readers[i]);
    }

    // Ожидаем завершения писателей по одному
    for (int i = 0; i < WRITERS_CNT; i++) {
        DWORD dw = WaitForSingleObject(writers[i], INFINITE);
        switch (dw) {
        case WAIT_OBJECT_0:
            printf("writer thread %d finished\n", writerIDs[i]);
            break;
        case WAIT_TIMEOUT:
            printf("waitThread timeout %lu\n", dw);
            break;
        case WAIT_FAILED:
            printf("waitThread failed %lu\n", dw);
            break;
        default:
            printf("unknown %lu\n", dw);
            break;
        }
        CloseHandle(writers[i]);
    }

    CloseHandle(canRead);
    CloseHandle(canWrite);
    CloseHandle(mutex);

    return 0;
}