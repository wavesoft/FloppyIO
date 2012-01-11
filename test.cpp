
#include <string.h>
#include <stdio.h>

#include "includes/disk.h"

int main(int argc, char** argv) {

    fpio::disk disk("./floppy.dsk", fpio::O_CREATE);

    size_t dsk_size = sizeof(*disk.map);
    char*  str_buf = new char[dsk_size];

    strcpy(disk.map->cBufferIn, "How are you?");
    memcpy(str_buf, disk.map, dsk_size);

    fwrite(str_buf, dsk_size, 1, stdout);

    delete[] str_buf;

    return 0;
}
