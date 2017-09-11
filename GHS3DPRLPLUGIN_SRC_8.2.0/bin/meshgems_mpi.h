/*
 * Copyright 2009-2013 Distene SAS
 *
 */

#ifndef __MESHGEMS_MPI_H__
#define __MESHGEMS_MPI_H__

#define MESHGEMS_MPI_SUCCESS 0
#define MESHGEMS_MPI_ERR 1

#define MESHGEMS_MPI_ANY_SOURCE -1

/* Declare these as int rather than enum to be able to create dynamic types/operations */

typedef int meshgems_mpi_datatype;
typedef int meshgems_mpi_op;

#define meshgems_mpi_datatype_none 0
#define meshgems_mpi_datatype_i4 1
#define meshgems_mpi_datatype_i8 2
#define meshgems_mpi_datatype_r4 3
#define meshgems_mpi_datatype_r8 4
#define meshgems_mpi_datatype_enum_count 5

#define meshgems_mpi_op_none 0
#define meshgems_mpi_op_max 1
#define meshgems_mpi_op_sum 2
#define meshgems_mpi_op_enum_count 3

int meshgems_mpi_type_new(int count, int *array_of_blocklengths, long *array_of_displacements,
                           meshgems_mpi_datatype *array_of_types, meshgems_mpi_datatype *newtype);
int meshgems_mpi_type_delete(meshgems_mpi_datatype datatype);

struct meshgems_mpi_handler_;
typedef struct meshgems_mpi_handler_ meshgems_mpi_handler;

meshgems_mpi_handler *meshgems_mpi_handler_new(void);
void meshgems_mpi_handler_delete(meshgems_mpi_handler *handler);

int meshgems_mpi_init(int *argc, char ***argv);
int meshgems_mpi_rank(int *r);
int meshgems_mpi_size(int *n);
int meshgems_mpi_finalize(void);

int meshgems_mpi_send(void* buffer, int count, meshgems_mpi_datatype datatype, int dest, int tag);
int meshgems_mpi_isend(void* buffer, int count, meshgems_mpi_datatype datatype, int dest, int tag,
                       meshgems_mpi_handler *handler);
int meshgems_mpi_wait(meshgems_mpi_handler *handler);

int meshgems_mpi_recv(void* buffer, int count, meshgems_mpi_datatype, int src, int tag);

int meshgems_mpi_reduce(void *sendbuf, void *recvbuf, int count, meshgems_mpi_datatype datatype,
                        meshgems_mpi_op op, int root);

int meshgems_mpi_allreduce(void *sendbuf, void *recvbuf, int count, meshgems_mpi_datatype datatype,
                           meshgems_mpi_op op);

int meshgems_mpi_gather(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype, void *recvbuf,
                        int recvcount, meshgems_mpi_datatype recvtype, int root);

int meshgems_mpi_allgather(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype,
                           void *recvbuf, int recvcount, meshgems_mpi_datatype recvtype);

int meshgems_mpi_allgatherv(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype,
                            void *recvbuf, int *recvcount, int *displs, meshgems_mpi_datatype recvtype);

int meshgems_mpi_barrier(void);

int meshgems_mpi_abort(int errorcode);

#endif
