#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "../includes/floppyIO.h"

using namespace std;
using namespace fpio;

int main(int argc, char** argv) {

    floppyIO * fpio = new floppyIO ("/Users/icharala/floppy.img", O_SYNCHRONIZED | O_DEVICE | O_CLIENT | O_EXCEPTIONS | O_EXTENDED );

    fpio->syncTimeout=0;

    int sid = 0;
    if (argc > 1) {
        sid = atoi(argv[1]);
    }

    printf("IN Control byte @ %i\nOUT Control byte @ %i\nWaiting at stream %i\n", 
        fpio->layout.ofsControlIn,
        fpio->layout.ofsControlOut,
        sid);

    fpio->send(&cin, sid);

    return 0;
}

