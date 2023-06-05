#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../../gray-scott/simulation/writer.h"

Writer::Writer(const Settings &settings, const GrayScott &sim, MPI_Comm comm)
: settings(settings), fd(0), timestep_offset(0), comm(comm)
{
}

void Writer::open(const std::string &fname)
{
    fd = ::open(fname.c_str(), O_CREAT | O_WRONLY, 0644);

    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
}

void Writer::write(const void *buf, size_t size)
{
    ssize_t bytes_remaining = size;
    const char *ptr = reinterpret_cast<const char *>(buf);
    while (bytes_remaining > 0)
    {
        ssize_t bytes_written = ::write(fd, ptr, bytes_remaining);

        if (bytes_written == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        ptr += bytes_written;
        bytes_remaining -= bytes_written;
    }
}

void Writer::write(int step, const GrayScott &sim)
{
    if (!sim.size_x || !sim.size_y || !sim.size_z)
    {
        return;
    }

    std::vector<double> u = sim.u_noghost();
    std::vector<double> v = sim.v_noghost();

    // Size of this rank for this timestep
    size_t this_size = sizeof(double) * u.size() + sizeof(double) * v.size();
    // Total size of this timestep
    size_t this_timestep_size = 0;
    // Offset of this rank from the beginning of the timestep
    size_t rank_offset = 0;

    MPI_Exscan(&this_size, &rank_offset, 1, MPI_UNSIGNED_LONG, MPI_SUM, comm);
    MPI_Allreduce(&this_size, &this_timestep_size, 1, MPI_UNSIGNED_LONG,
                  MPI_SUM, comm);

    lseek(fd, timestep_offset + rank_offset, SEEK_SET);
    write(u.data(), sizeof(double) * u.size());
    write(v.data(), sizeof(double) * v.size());

    timestep_offset += this_timestep_size;
}

void Writer::close()
{
    if (fd)
    {
        ::close(fd);
        fd = 0;
    }
}
