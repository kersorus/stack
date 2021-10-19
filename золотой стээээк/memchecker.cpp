#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int mem_check(const void *ptr, const size_t byte_amount)
{
#ifndef NDEBUG
    int fd = open(".", O_TMPFILE | O_WRONLY);

    errno = 0;
    char buffer[byte_amount];
    write(fd, buffer, byte_amount);

    if (errno == EFAULT)
        return 0;
    else
        return 1;

    close(fd);
#else
    return 1;
#endif
}
