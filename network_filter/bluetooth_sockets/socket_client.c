#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
   struct sockaddr_rc addr = {0};
   int s, status;
   char dest[18] = "FF:FF:FF:00:00:00";

   s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

   addr.rc_family = AF_BLUETOOTH;
   addr.rc_channel = (uint8_t) 1;
   str2ba(dest, &addr.rc_bdaddr);

   printf("connecting...\n");
   status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

   if(status == 0)
   {
       status = write(s, "hello1", 6);
   }
   if(status < 0)
   {
      perror("uh oh");
   }
   close(s);
   return 0;
}
