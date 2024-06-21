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
    // uartwrite("\n", 1);
    // uartwrite("abc\n", 4);

    while (1) {
        uartwrite("P3!\n", 4);
    }

    // char buf[10];
    // while (uartread(buf, 1)) {
    //     uartwrite(buf, 1);
    // }

}