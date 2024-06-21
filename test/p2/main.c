int main()
{
    // int pid = getpid();

    // char buffer[100];
    // int idx = 0;
    // if (pid == 0) {
    //     buffer[idx++] = '0';
    // } else {
    //     while (pid) {
    //         buffer[idx++] = '0' + (pid % 10);
    //         pid /= 10;
    //     }
    // }
    // uartwrite(buffer, 100);
    // uartwrlite("\n", 1);
    // uartwrite("abc\n", 4);

    uartwrite("P2!\n", 4);

    int pid = fork();
    if (pid > 0) {
        char buffer[100];
        int idx = 0;
        if (pid == 0) {
            buffer[idx++] = '0';
        } else {
            while (pid) {
                buffer[idx++] = '0' + (pid % 10);
                pid /= 10;
            }
        }
        // uartwrite("child pid: ", 12);
        // uartwrite(buffer, 100);
        // uartwrite("\n", 1);
        while (1) {
            uartwrite("parent\n", 7);
        }
    } else if (pid == 0) {
        exec("p3.img", 0);
        // while (1) {
        //     uartwrite("child\n", 6);
        // }
    }
}