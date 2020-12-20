#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "periodic_work_queue_ioctl_common.h"

int main(int argc, char* argv[])
{
    int experiation_time = 300;

    int fd = open("/dev/periodic_work_queue_module", O_RDWR);
    if (fd == -1)
    {
        printf("could not get desciptor on driver\n");
        exit(EXIT_FAILURE);
    }

    printf("starting work queue...\n");
    if (ioctl(fd, PWQ_START_IOCTL, &experiation_time) < 0)
    {
        printf("failed send start ioctl\n");
        exit(EXIT_FAILURE);
    }

    printf("sleeping...\n");

    sleep(5);

    experiation_time = 10000;
    printf("setting work queue delay...\n");
    if (ioctl(fd, PWQ_SET_DELAY_IOCTL, &experiation_time) < 0)
    {
        printf("failed send experiation time ioctl\n");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return EXIT_SUCCESS;
}
