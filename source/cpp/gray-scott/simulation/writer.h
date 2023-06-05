#ifndef __WRITER_H__
#define __WRITER_H__

#include <mpi.h>

#include "../../gray-scott/simulation/gray-scott.h"
#include "../../gray-scott/simulation/settings.h"

class Writer
{
public:
    Writer(const Settings &settings, const GrayScott &sim, MPI_Comm comm);
    void open(const std::string &fname);
    void write(const void *buf, size_t size);
    void write(int step, const GrayScott &sim);
    void close();

protected:
    Settings settings;
    int fd;
    size_t timestep_offset;
    MPI_Comm comm;
};

#endif
