/*
 * Copyright 2009-2013 Distene SAS
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "meshgems_mpi.h"

#include <mpi.h>

static MPI_Datatype *meshgems_mpi_datatype_map = NULL;
static MPI_Op *meshgems_mpi_op_map = NULL;
static int meshgems_mpi_datatype_count = 0;
static int meshgems_mpi_op_count = 0;

struct meshgems_mpi_handler_
{
  MPI_Request rq;
};

/* note : MPI standard states that a MPI_Datatype is a simple type (pointer or int) */

static inline int meshgems_mpi_find_type(meshgems_mpi_datatype datatype, MPI_Datatype *odt)
{
  if(meshgems_mpi_datatype_map && datatype > 0 && datatype <= meshgems_mpi_datatype_count) {
    *odt = meshgems_mpi_datatype_map[datatype];
    return MESHGEMS_MPI_SUCCESS;
  }

  return MESHGEMS_MPI_ERR;
}

int meshgems_mpi_type_new(int count, int *array_of_blocklengths, long *array_of_displacements,
    meshgems_mpi_datatype *array_of_types, meshgems_mpi_datatype *newtype)
{
  MPI_Datatype nt, *aot;
  MPI_Aint *aod;
  int ret, r, i;
  void *t;

  ret = MESHGEMS_MPI_SUCCESS;

  aot = (MPI_Datatype *) calloc(count, sizeof(MPI_Datatype));
  aod = (MPI_Aint *) calloc(count, sizeof(MPI_Aint));
  if(!aot || !aod) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  for(i = 0;i < count;i++) {
    /*
     r = meshgems_mpi_find_type(array_of_types[i], aot+i);
     if(r != MESHGEMS_MPI_SUCCESS){
     ret  = r;
     goto out;
     }
     */
    aot[i] = meshgems_mpi_datatype_map[array_of_types[i]];
    aod[i] = (MPI_Aint) array_of_displacements[i];
  }

  r = MPI_Type_create_struct(count, array_of_blocklengths, aod, aot, &nt);
  /* With MPI1 you can use :
   r = MPI_Type_struct(count, array_of_blocklengths,
   aod, aot, &nt);
   */

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }
  r = MPI_Type_commit(&nt);
  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  meshgems_mpi_datatype_count++;
  t = realloc(meshgems_mpi_datatype_map, (meshgems_mpi_datatype_count + 1) * sizeof(MPI_Datatype));
  if(!t) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }
  meshgems_mpi_datatype_map = (MPI_Datatype *) t;
  meshgems_mpi_datatype_map[meshgems_mpi_datatype_count] = nt;
  *newtype = meshgems_mpi_datatype_count;

  out: if(aod)
    free(aod);
  if(aot)
    free(aot);

  return ret;
}

int meshgems_mpi_type_delete(meshgems_mpi_datatype datatype)
{
  /* no op for now */
  return MESHGEMS_MPI_SUCCESS;
}

int meshgems_mpi_init(int *argc, char ***argv)
{
  int ret;

  ret = MESHGEMS_MPI_SUCCESS;

  meshgems_mpi_datatype_map = calloc(meshgems_mpi_datatype_enum_count, sizeof(MPI_Datatype));
  meshgems_mpi_op_map = calloc(meshgems_mpi_op_enum_count, sizeof(MPI_Op));
  if(!meshgems_mpi_datatype_map || !meshgems_mpi_op_map) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }
  meshgems_mpi_datatype_count = meshgems_mpi_datatype_enum_count - 1;
  meshgems_mpi_op_count = meshgems_mpi_op_enum_count - 1;

  if(sizeof(int) == 4) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_i4] = MPI_INT;
  } else if(sizeof(int) == 4) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_i4] = MPI_SHORT;
  } else {
    fprintf(stderr, "Unable to find mpi type for 4 bytes integer\n");
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  if(sizeof(int) == 8) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_i8] = MPI_INT;
  } else if(sizeof(long) == 8) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_i8] = MPI_LONG;
  } else if(sizeof(long long) == 8) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_i8] = MPI_LONG_LONG;
  } else {
    fprintf(stderr, "Unable to find mpi type for 8 bytes integer\n");
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  if(sizeof(float) == 4) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_r4] = MPI_FLOAT;
  } else {
    fprintf(stderr, "Unable to find mpi type for 4 bytes real\n");
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  if(sizeof(double) == 8) {
    meshgems_mpi_datatype_map[meshgems_mpi_datatype_r8] = MPI_DOUBLE;
  } else {
    fprintf(stderr, "Unable to find mpi type for 8 bytes real\n");
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  meshgems_mpi_op_map[meshgems_mpi_op_max] = MPI_MAX;
  meshgems_mpi_op_map[meshgems_mpi_op_sum] = MPI_SUM;

  MPI_Init(argc, argv);

  out:

  if(ret != MESHGEMS_MPI_SUCCESS) {
    if(meshgems_mpi_datatype_map)
      free(meshgems_mpi_datatype_map);
    if(meshgems_mpi_op_map)
      free(meshgems_mpi_op_map);
  }

  return ret;
}

int meshgems_mpi_rank(int *r)
{
  MPI_Comm_rank(MPI_COMM_WORLD, r);

  return MESHGEMS_MPI_SUCCESS;
}

int meshgems_mpi_size(int *n)
{
  MPI_Comm_size(MPI_COMM_WORLD, n);

  return MESHGEMS_MPI_SUCCESS;
}

int meshgems_mpi_finalize(void)
{
  MPI_Finalize();

  return MESHGEMS_MPI_SUCCESS;
}

meshgems_mpi_handler *meshgems_mpi_handler_new(void)
{
  meshgems_mpi_handler *handler;

  handler = calloc(1, sizeof(meshgems_mpi_handler));

  return handler;
}

void meshgems_mpi_handler_delete(meshgems_mpi_handler *handler)
{
  if(handler)
    free(handler);
}

int meshgems_mpi_send(void* buffer, int count, meshgems_mpi_datatype datatype, int dest, int tag)
{
  int ret;
  int r;
  MPI_Datatype dtt;

  dtt = meshgems_mpi_datatype_map[datatype];

  r = MPI_Send(buffer, count, dtt, dest, tag, MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_isend(void* buffer, int count, meshgems_mpi_datatype datatype, int dest, int tag,
    meshgems_mpi_handler *handler)
{
  int ret;
  int r;
  MPI_Datatype dtt;

  dtt = meshgems_mpi_datatype_map[datatype];

  r = MPI_Isend(buffer, count, dtt, dest, tag, MPI_COMM_WORLD, &(handler->rq));

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_wait(meshgems_mpi_handler *handler)
{
  int ret;
  int r;

  r = MPI_Wait(&(handler->rq), MPI_STATUS_IGNORE );

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_recv(void* buffer, int count, meshgems_mpi_datatype datatype, int src, int tag)
{
  int ret;
  int r;
  MPI_Datatype dtt;

  dtt = meshgems_mpi_datatype_map[datatype];

  if(src == MESHGEMS_MPI_ANY_SOURCE) {
    src = MPI_ANY_SOURCE;
  }

  r = MPI_Recv(buffer, count, dtt, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_reduce(void *sendbuf, void *recvbuf, int count, meshgems_mpi_datatype datatype,
    meshgems_mpi_op op, int root)
{
  int ret;
  int r;
  MPI_Datatype dtt;
  MPI_Op mop;

  dtt = meshgems_mpi_datatype_map[datatype];
  mop = meshgems_mpi_op_map[op];

  r = MPI_Reduce(sendbuf, recvbuf, count, dtt, mop, root, MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_allreduce(void *sendbuf, void *recvbuf, int count, meshgems_mpi_datatype datatype,
    meshgems_mpi_op op)
{
  int ret;
  int r;
  MPI_Datatype dtt;
  MPI_Op mop;

  dtt = meshgems_mpi_datatype_map[datatype];
  mop = meshgems_mpi_op_map[op];

  r = MPI_Allreduce(sendbuf, recvbuf, count, dtt, mop, MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_gather(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype, void *recvbuf,
    int recvcount, meshgems_mpi_datatype recvtype, int root)
{
  int ret;
  int r;
  MPI_Datatype sdtt, rdtt;

  sdtt = meshgems_mpi_datatype_map[sendtype];
  rdtt = meshgems_mpi_datatype_map[recvtype];

  r = MPI_Gather(sendbuf, sendcount, sdtt, recvbuf, recvcount, rdtt, root, MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_allgather(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype,
    void *recvbuf, int recvcount, meshgems_mpi_datatype recvtype)
{
  int ret;
  int r;
  MPI_Datatype sdtt, rdtt;

  ret = MESHGEMS_MPI_SUCCESS;

  sdtt = meshgems_mpi_datatype_map[sendtype];
  rdtt = meshgems_mpi_datatype_map[recvtype];

  r = MPI_Allgather(sendbuf, sendcount, sdtt, recvbuf, recvcount, rdtt, MPI_COMM_WORLD);
  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
    goto out;
  }

  out:

  return ret;
}

int meshgems_mpi_allgatherv(void *sendbuf, int sendcount, meshgems_mpi_datatype sendtype,
    void *recvbuf, int *recvcount, int *displs, meshgems_mpi_datatype recvtype)
{
  int ret;
  int r;
  MPI_Datatype sdtt, rdtt;

  sdtt = meshgems_mpi_datatype_map[sendtype];
  rdtt = meshgems_mpi_datatype_map[recvtype];

  r = MPI_Allgatherv(sendbuf, sendcount, sdtt, recvbuf, recvcount, displs, rdtt, MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_barrier(void)
{
  int ret;
  int r;

  r = MPI_Barrier(MPI_COMM_WORLD);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}

int meshgems_mpi_abort(int errorcode)
{
  int ret;
  int r;

  r = MPI_Abort(MPI_COMM_WORLD, errorcode);

  if(r != MPI_SUCCESS) {
    ret = MESHGEMS_MPI_ERR;
  } else {
    ret = MESHGEMS_MPI_SUCCESS;
  }

  return ret;
}
