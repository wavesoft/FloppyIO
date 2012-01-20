#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../includes/floppyIO.h"

using namespace fpio;

int main(int argc, char** argv) {

    floppyIO * fpio = new floppyIO ("/dev/fd0", O_NORESET | O_EXCEPTIONS | O_SYNCHRONIZED | O_EXTENDED );

    fpio->syncTimeout=0;

    int sid = 0;
    if (argc > 1) {
        sid = atoi(argv[1]);
    }

    fprintf(stderr, "IN Control byte @ %i\nOUT Control byte @ %i\nWaiting at stream %i\n", 
        fpio->layout.ofsControlIn,
        fpio->layout.ofsControlOut,
        sid);
    
    fpio->receive(&cout, sid);

    return 0;
}

