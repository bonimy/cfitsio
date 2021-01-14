#include <vector>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "fitsio.h"

typedef std::vector<char*> fits_vector;

int writevecimage();

int main() {
    int status;
    status = writevecimage();
    fits_report_error(stderr, status);
    return status;
}

int vector_truncate(LONGLONG filesize, void* userp) {
    auto vector = static_cast<fits_vector*>(userp);
    vector->resize(filesize);
    return 0;
}

int vector_size(LONGLONG* sizex, void* userp) {
    auto vector = static_cast<fits_vector*>(userp);
    *sizex = static_cast<LONGLONG>(vector->size());
    return 0;
}

int vector_read(void* buffer, long nbytes, LONGLONG* offset, void* userp) {
    auto vector = static_cast<fits_vector*>(userp);
    if (*offset + nbytes > vector->size()) return (END_OF_FILE);
    memcpy(buffer, vector->data() + *offset, nbytes);
    *offset += nbytes;
    return 0;
}

int vector_write(void* buffer, long nbytes, LONGLONG* offset, void* userp) {
    auto vector = static_cast<fits_vector*>(userp);
    if ((size_t)(*offset + nbytes) > vector->size())
        vector->resize((((*offset + nbytes - 1) / 2880) + 1) * 2880);
    memcpy(vector->data() + *offset, buffer, nbytes);
    *offset += nbytes;
    return (0);
}

int writevecimage() {
    fitsfile* fptr = NULL;
    int status = 0;
    long naxes[2] = {300, 200};

    std::vector<char> buffer;
    fits_create_userfile(&fptr, nullptr, vector_truncate, nullptr, vector_size, nullptr,
                         nullptr, vector_read, vector_write,
                         static_cast<void*>(&buffer), &status);
    fits_create_img(fptr, USHORT_IMG, sizeof(naxes) / sizeof(long), naxes, &status);
    fits_close_file(fptr, &status);

    std::fstream fout;
    fout.open("vecexample.fit", std::ios::binary | std::ios::out);
    fout.write(buffer.data(), buffer.size());
    fout.close();

    return status;
}
