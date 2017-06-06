#include <stdlib.h>
#include <stdio.h>

#include "gr.h"

int main(void) {
    double x[3] = {0.0, 0.5, 1.0};
    double y[3] = {0.1, 0.25, 0.9};
    int n = sizeof(x) / sizeof(x[0]);
    void *handle;

    printf("sending data...");

    handle = gr_openmeta(GR_TARGET_SOCKET, "localhost", 8001);
    if (handle == NULL) {
        fprintf(stderr, "Sender could not be created\n");
        return -1;
    }

    gr_sendmeta(handle, "s(data:s(x:npD,y:pD)", n, x, y);
    gr_sendmeta(handle, "color:ddd)", 1.0, 0.0, 0.5);

    printf("\tsent\n");

    return 0;
}
