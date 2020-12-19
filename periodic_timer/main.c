#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "periodic_timer_ioctl_common.h"

int main(int argc, char* argv[])
{
    int experiation_time = 200000;

    int fd = open("/dev/periodic_timer_module", O_RDWR);
    if (fd == -1)
    {
        printf("could not get desciptor on driver\n");
        exit(EXIT_FAILURE);
    }

    printf("starting timer...\n");
    if (ioctl(fd, PT_START_IOCTL, &experiation_time) < 0)
    {
        printf("failed send start ioctl\n");
        exit(EXIT_FAILURE);
    }

    printf("sleeping...\n");

    sleep(5);

    printf("setting timer expiration...\n");
    if (ioctl(fd, PT_SET_EXPIRATION_IOCTL, &experiation_time) < 0)
    {
        printf("failed send experiation time ioctl\n");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return EXIT_SUCCESS;
}
