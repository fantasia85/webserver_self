/* main */

#include "Eventloop.h"
#include "Server.h"
#include <string>

int main(int argc, char *argv[])
{
    /* open 4 threads */
    int threadnum = 4;
    int port = 80;
    /*  */

    int opt;
    const char *str = "t:p";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch(opt) {
            case 't':
                threadnum = atoi(optarg);
                break;
            /* case "l": */
            case 'p':
                port = atoi(optarg);
                break;
            default:
                break;
        }
    }

    /*    */

#ifndef _PTHREADS
    /*     */
#endif

    Eventloop mainloop;
    Server myhttpserver(&mainloop, threadnum, port);
    myhttpserver.start();
    mainloop.loop();
    return 0;
}