#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>
#include <pthread.h>

#include <stdio.h>

#define LOCKFILE "/var/run/daemon.pid"
#define CONFFILE "/etc/daemon.conf"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)    // read_user, write_user, read_group, read_others

/*Integer or structure type of an object used to
represent sets of signals.*/
sigset_t mask;

// Инициализация процесса-демона
void daemonize(const char *cmd)
{
    int                 i, fd0, fd1, fd2;
    pid_t               pid;
    struct rlimit       rl;
    struct sigaction    sa;

    /*
     * Сброс маску создания файлов в значение 0
     */
    umask(0);

    /*
     * Получить максимально возможный номер дескриптора файла.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
        err_quit("%s: невозможно получить максимальный номер дескриптора ", cmd);

    printf("TTY: %s", ctermid(NULL));

    /*
     * Стать лидером нового сеанса, чтобы утратить управляющий терминал.
     */
    if ((pid = fork()) == -1)
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0) /* родительский процесс */
    {
        // sleep(15);
        exit(0);
    }

    printf("PID = %d\n", getpid());

    /*
     * Создаем новый сеанс
     */
    if (setsid() == -1)
    {
        perror("Ошибка setsid");
        exit(EXIT_FAILURE);
    }

    /*
     * Обеспечить невозможность обретения управляющего терминала в будущем.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        err_quit("%s:  невозможно игнорировать сигнал SIGHUP", cmd);
        
    // этого в линукс делать не надо
    // if ((pid = fork()) < 0)
    //     err_quit("%s: ошибка вызова функции fork", cmd);
    // else if (pid != 0) /* родительский процесс */
    //     exit(0);
    
    /*
     * Назначить корневой каталог текущим рабочим каталогом,
     * чтобы впоследствии можно было отмонтировать файловую систему.
     */
    if (chdir("/") == -1)
        err_quit("%s:  невозможно сделать текущим рабочим каталогом  /", cmd);

    /*
     * Закрыть все открытые файловые дескрипторы.
     */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024; // макс число файловых дескрипторов которые могут быть открыты
    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    // int fd = open("/dev/pts/5", O_WRONLY && !O_NOCTTY);
    // if (fd == -1)
    // {
    //     syslog(LOG_ERR, "Error open ");
    //     exit(EXIT_FAILURE);
    // }
    // write(fd, "qweqwe", 7);

    /*
     * Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
     */
    fd0 = open("/dev/pts/4", O_RDWR | O_NOCTTY);
    fd1 = dup(0);
    fd2 = dup(0);
    printf("qweqweqweqwe\n");
    /*
     * Инициализировать файл журнала.
     */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d",
               fd0, fd1, fd2);
        exit(1);
    }
}

// Установка блокировки для записи на весь файл
int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;        // Type of lock - write lock
    fl.l_start = 0;             // is the starting offset for the lock, and is interpreted relative to either
    fl.l_whence = SEEK_SET;     // the start of the file
    fl.l_len = 0;               // specifies the number of bytes to be locke
    /*if l_len == 0 lock all bytes starting at the location specified by l_whence and l_start 
    through to the end of file, no matter how large the file grows.*/

    /*F_SETLK: Acquire a lock (when l_type is F_RDLCK or F_WRLCK) or release a lock (when l_type is F_UNLCK) 
    on the bytes specified by the l_whence, l_start, and l_len fields of lock. */
    return (fcntl(fd, F_SETLK, &fl));   // manipulate file descriptor
}


int already_running(void)
{
    int fd;
    int rc;
    int conffd;
    char buf[16];
    char conf_buf[256];
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(LOCKFILE, O_RDWR | O_CREAT, perms);
    if (fd == -1)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    if ((conffd = open(CONFFILE, O_RDWR | O_CREAT, perms)) == -1) // username userid
    {
        syslog(LOG_ERR, "ошибка открытия файла %s", CONFFILE);
        exit(EXIT_FAILURE);
    }
    ftruncate(conffd, 0);
    sprintf(conf_buf, "%ld\n%s", (long) getuid(), getlogin());
    // sprintf(conf_buf, "%ld", (long) getuid());
    rc = write(conffd, conf_buf, strlen(conf_buf) + 1);
    if (rc == -1) {
        syslog(LOG_ERR, "ошибка записис в файл %s", CONFFILE);
        exit(EXIT_FAILURE);
    }

    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    ftruncate(fd, 0);   
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}


void reread(void)
{
    FILE *fd;
    int pid;
    char login[50];
    int userid;
    
    if ((fd = fopen(CONFFILE, "r")) == NULL)
    {
        syslog(LOG_INFO, "Ошибка fopen " CONFFILE "");
        exit(EXIT_FAILURE);
    }

    fscanf(fd, "%d\n", &userid);
    fscanf(fd, "%s\n", login);
    fclose(fd);

    syslog(LOG_INFO, "UserName: %s", login);
    syslog(LOG_INFO, "UserID: %d", userid);
}

void *thr_fn(void *arg)
{
    int err, signo;

    for (;;)
    {
        /* sigwait() function suspends execution of the calling thread
       until one of the signals specified in the signal set set becomes
       pending(рассматриваться).  ??? For a signal to become pending, it must first be
       blocked with sigprocmask(2)???.  The function accepts the signal
       (removes it from the pending list of signals), and returns the
       signal number in sig. */
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "sigwait failed");
            exit(EXIT_FAILURE);
        }

        switch (signo)
        {
        case SIGHUP:
            syslog(LOG_INFO, "Re-reading configuration file");
            reread();
            break;

        case SIGTERM:
            syslog(LOG_INFO, "got SIGTERM; exiting");
            exit(0);

        default:
            syslog(LOG_INFO, "unexpected signal %d\n", signo);
        }
    }
}


#include <sys/ioctl.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    char *cmd;

    // cmd - имя исполняемого файла (без пути)
    /* char *strrchr(const char *s, int c); 
        возвращает указатель на местонахождение последнего символа равного символу c в строке s. */
    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    /*
     * Стать лидером нового сеанса, чтобы утратить управляющий терминал.
     */
    if ((pid = fork()) == -1)
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0) /* родительский процесс */
    {
        // sleep(15);
        exit(0);
    }

    printf("PID = %d\n", getpid());

    /*
     * Создаем новый сеанс
     */
    if (setsid() == -1)
    {
        perror("Ошибка setsid");
        exit(EXIT_FAILURE);
    }

    int fd = open("/dev/pts/2", O_WRONLY);
    if (fd == -1)
    {
        syslog(LOG_ERR, "Error open ");
        exit(EXIT_FAILURE);
    }
    ioctl(fd, TIOCSCTTY, 1);
    write(fd, "qweqwe", 7);

    while(1);
}

// int main(int argc, char *argv[])
// {
//     int err;
//     pthread_t tid;
//     char *cmd;
//     struct sigaction sa;

//     // cmd - имя исполняемого файла (без пути)
//     /* char *strrchr(const char *s, int c); 
//         возвращает указатель на местонахождение последнего символа равного символу c в строке s. */
//     if ((cmd = strrchr(argv[0], '/')) == NULL)
//         cmd = argv[0];
//     else
//         cmd++;

//     /*
//      * Перейти в режим демона.
//      */
//     daemonize(cmd);

    

//     /*
//      * Убедиться, что ранее не была запущена другая копия демона.
//      */
//     if (already_running())
//     {
//         syslog(LOG_ERR, "демон уже запущен");
//         exit(EXIT_FAILURE);
//     }

//     // // до этого момента SIGHUP игнорируется
//     // /*
//     //  * Восстановить действие по умолчанию для сигнала SIGHUP
//     //  * и заблокировать все сигналы.
//     //  */
//     // sa.sa_handler = SIG_DFL;
//     // sigemptyset(&sa.sa_mask);
//     // sa.sa_flags = 0;
//     // if (sigaction(SIGHUP, &sa, NULL) == -1)
//     //     err_quit("%s:  невозможно восстановить действие SIG_DFL для SIGHUP");
//     // sigfillset(&mask);          // initialize and fill a signal set

//     // /*
//     // The pthread_sigmask() function is just like sigprocmask(2), with
//     //                     the difference that its use in multithreaded programs is
//     //                     explicitly specified by POSIX.1.
//     // sigprocmask() is used to fetch and/or change the signal mask of
//     //             the calling thread.  The signal mask is the set of signals whose
//     //             delivery is currently blocked for the caller
//     // SIG_BLOCK The set of blocked signals is the union of the current set
//     //           and the set argument.
//     // */
//     // if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
//     //     err_exit(err, "ошибка выполнения операции SIG_BLOCK");

//     // /*
//     //  * Создать поток для обработки SIGHUP и SIGTERM.
//     //  */
//     // err = pthread_create(&tid, NULL, thr_fn, 0);
//     // if (err == -1)
//     //     err_exit(err, "can't create thread");

//     /*
//      * Proceed with the rest of the daemon.
//      */
//     while (1)
//     {
//         time_t cur_time = time(NULL);
//         syslog(LOG_NOTICE, "Time: %s", ctime(&cur_time));
//         sleep(10);
//         printf("e\n");
//     }

//     return 0;
// }