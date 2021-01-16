#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
    struct sockaddr_rc loc_addr = {0};
    struct sockaddr_rc rem_addr = {0};
    char buf[1024] = {0};
    char buf2[1024] = {0};
    char source[18] = "00:00:00:00:00:01";
    int s, client, byets_read;
    socklen_t opt = sizeof(rem_addr);
    int bytes_read;

    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    loc_addr.rc_family = AF_BLUETOOTH;
    //str2ba(source, &loc_addr.rc_bdaddr);
    loc_addr.rc_bdaddr = *BDADDR_LOCAL; // BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t)1;
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    ba2str(&loc_addr.rc_bdaddr, buf2);
    printf("addr %s\n", buf2);

    listen(s, 1);

    printf("accepting...");
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str(&rem_addr.rc_bdaddr, buf);
    printf("string addr %s\n", buf);

    memset(buf, 0, sizeof(buf));

    bytes_read = read(client, buf, sizeof(buf));
    if(bytes_read > 0)
    {
       printf("%s \n", buf);
    }

    //sendto(s, buf, 2, MSG_NOSIGNAL, (struct sockaddr *)&rem_addr, opt);

    close(client);
    close(s);
    return 0;
}
