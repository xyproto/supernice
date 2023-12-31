#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

static const char* version_string = "supernice 1.0.0";

enum {
    IOPRIO_CLASS_NONE,
    IOPRIO_CLASS_RT,
    IOPRIO_CLASS_BE,
    IOPRIO_CLASS_IDLE,
};
enum {
    IOPRIO_WHO_PROCESS = 1,
    IOPRIO_WHO_PGRP,
    IOPRIO_WHO_USER,
};

static int IOPRIO_CLASS_SHIFT = 13;
static int EX_EXEC_FAILED = 126; // Program located, but not usable
static int EX_EXEC_ENOENT = 127; // Could not find program to exec

static inline int flush_standard_stream(FILE* stream)
{
    errno = 0;
    if (ferror(stream) != 0 || fflush(stream) != 0) {
        return (errno == EBADF) ? 0 : EOF;
    }
    /*
     * Calling fflush is not sufficient on some filesystems like e.g. NFS,
     * which may defer the actual flush until close. Calling fsync would help
     * solve this, but would probably result in a performance hit. Thus, we
     * work around this issue by calling close on a dup'd file descriptor from
     * the stream.
     */
    int fd = fileno(stream);
    if (fd < 0 || (fd = dup(fd)) < 0 || close(fd) != 0) {
        return (errno == EBADF) ? 0 : EOF;
    }
    return 0;
}

/* Meant to be used atexit(close_stdout); */
static inline void close_stdout(void)
{
    if (flush_standard_stream(stdout) != 0 && !(errno == EPIPE)) {
        if (errno) {
            warn("write error");
        } else {
            warnx("write error");
        }
    } else if (flush_standard_stream(stderr) == 0) {
        return;
    }
    _exit(EXIT_FAILURE);
}

static inline int64_t strtos64_or_err(const char* str, const char* errmesg)
{
    char* end = NULL;
    int64_t num;
    errno = 0;
    if (str == NULL || *str == '\0') {
        if (errno == ERANGE) {
            err(EXIT_FAILURE, "%s: '%s'", errmesg, str);
        }
        errx(EXIT_FAILURE, "%s: '%s'", errmesg, str);
    }
    num = strtoimax(str, &end, 10);
    if (errno || str == end || (end && *end)) {
        if (errno == ERANGE) {
            err(EXIT_FAILURE, "%s: '%s'", errmesg, str);
        }
        errx(EXIT_FAILURE, "%s: '%s'", errmesg, str);
    }
    return num;
}

static inline int32_t strtos32_or_err(const char* str, const char* errmesg)
{
    int64_t num = strtos64_or_err(str, errmesg);
    if (num < INT32_MIN || num > INT32_MAX) {
        errno = ERANGE;
        err(EXIT_FAILURE, "%s: '%s'", errmesg, str);
    }
    return (int32_t)num;
}

static inline unsigned long IOPRIO_PRIO_MASK()
{
    return (1UL << IOPRIO_CLASS_SHIFT) - 1;
}

static inline unsigned long IOPRIO_PRIO_CLASS(unsigned long mask)
{
    return mask >> IOPRIO_CLASS_SHIFT;
}

static inline unsigned long IOPRIO_PRIO_DATA(unsigned long mask)
{
    return mask & IOPRIO_PRIO_MASK();
}

static inline unsigned long IOPRIO_PRIO_VALUE(unsigned long class, unsigned long data)
{
    return ((class << IOPRIO_CLASS_SHIFT) | data);
}

static void ioprio_setid(int which, int ioclass, int data, int who, bool tolerant)
{
    const int rc = syscall(SYS_ioprio_set, who, which, IOPRIO_PRIO_VALUE(ioclass, data));
    if (rc == -1 && !tolerant) {
        err(EXIT_FAILURE, "ioprio_set failed");
    }
}

static void __attribute__((__noreturn__)) usage(void)
{
    fputs("\nUsage:\n"
          " supernice <command>\n\n"
          "Set the I/O-scheduling class, priority and niceness when launching a process.\n\n"
          "Options:\n"
          " -h, --help             display this help\n"
          " -V, --version          display version\n",
        stdout);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    bool tolerant = false;
    int c = 0, ioclass = IOPRIO_CLASS_IDLE, niceness = 10;

    static const struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { NULL, 0, NULL, 0 }
    };
    atexit(close_stdout);

    while ((c = getopt_long(argc, argv, "Vh", longopts, NULL)) != EOF)
        switch (c) {
        case 'V':
            printf("%s\n", version_string);
            exit(EXIT_SUCCESS);
        case 'h':
            usage();
        default:
            fprintf(stderr, "Try 'supernice --help' for more information.\n");
            exit(EXIT_FAILURE);
        }

	if (argc < 2) {
		fprintf(stderr, "Provide a command as the first argument.\n");
		exit(EXIT_FAILURE);
	}

    // supernice COMMAND
    ioprio_setid(0, ioclass, 7, IOPRIO_WHO_PROCESS, tolerant);
    setpriority(0, PRIO_PROCESS, niceness);
    execvp(argv[optind], &argv[optind]);
    err(errno == ENOENT ? EX_EXEC_ENOENT : EX_EXEC_FAILED, "failed to execute %s", argv[optind]);
    return EXIT_SUCCESS;
}
