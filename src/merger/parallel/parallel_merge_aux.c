/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                 MPItrace                                  *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $Source: /home/paraver/cvs-tools/mpitrace/fusion/src/merger/parallel/parallel_merge_aux.c,v $
 | 
 | @last_commit: $Date: 2009/05/28 14:40:44 $
 | @version:     $Revision: 1.17 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "common.h"

static char UNUSED rcsid[] = "$Id: parallel_merge_aux.c,v 1.17 2009/05/28 14:40:44 harald Exp $";

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
# ifdef HAVE_FOPEN64
#  define __USE_LARGEFILE64
# endif
# include <stdio.h>
#endif

#include <mpi.h>
#include "mpi-tags.h"
#include "mpi-aux.h"
#include "parallel_merge_aux.h"
#include "mpi_comunicadors.h"

#include "mpi_prv_events.h"
#include "pthread_prv_events.h"
#include "omp_prv_events.h"
#include "misc_prv_events.h"
#include "addr2info.h"

static struct ForeignRecv_t **myForeignRecvs;
static int *myForeignRecvs_count;
static char **myForeignRecvs_used;

#define FOREIGN_RECV_RESIZE_STEP ((1024*1024)/sizeof(struct ForeignRecv_t))
#define COMMUNICATORS_RESIZE_STEP ((1024*1024)/sizeof(struct Communicator_t))
#define PENDING_COMM_RESIZE_STEP ((1024*1024)/sizeof(struct PendingCommunication_t))

void AddCommunicator (int ptask, int task, int type, int id, int ntasks, int *tasks)
{
	int count = Communicators.count;

	if (Communicators.count == Communicators.size)
	{
		Communicators.size += COMMUNICATORS_RESIZE_STEP;
		Communicators.comms = (struct Communicator_t*)
			realloc (Communicators.comms,
			Communicators.size*sizeof(struct Communicator_t));
	}
	Communicators.comms[count].ptask = ptask;
	Communicators.comms[count].task = task;
	Communicators.comms[count].type = type;
	Communicators.comms[count].id = id;
	Communicators.comms[count].ntasks = ntasks;
	if (MPI_COMM_WORLD_ALIAS != type && MPI_COMM_SELF_ALIAS != type)
	{
		int i;

		Communicators.comms[count].tasks = (int*) malloc (sizeof(int)*ntasks);
		if (NULL == Communicators.comms[count].tasks)
		{
			fprintf (stderr, "mpi2prv: ERROR! Unable to store communicator information\n");
			fflush (stderr);
			exit (-1);
		}
		for (i = 0; i < ntasks; i++)
			Communicators.comms[count].tasks[i] = tasks[i];
	}
	else
		Communicators.comms[count].tasks = NULL;

	Communicators.count++;
}

void InitCommunicators(void)
{
	Communicators.size = Communicators.count = 0;
	Communicators.comms = NULL;
}

void AddPendingCommunication (int descriptor, off_t offset, int tag, int task_r,
	int task_s)
{
	int count = PendingComms.count;

	if (PendingComms.count == PendingComms.size)
	{
		PendingComms.size += PENDING_COMM_RESIZE_STEP;
		PendingComms.data = (struct PendingCommunication_t*) 
			realloc (PendingComms.data, 
			PendingComms.size*sizeof(struct PendingCommunication_t));
	}
	PendingComms.data[count].offset = offset;
	PendingComms.data[count].descriptor = descriptor;
	PendingComms.data[count].recver = task_r;
	PendingComms.data[count].sender = task_s;
	PendingComms.data[count].tag = tag;
	PendingComms.data[count].match = 0;
	PendingComms.count++;
}

void InitPendingCommunication (void)
{
	PendingComms.size = PendingComms.count = 0;
	PendingComms.data = NULL;
}

void AddForeignRecv (UINT64 physic, UINT64 logic, int tag, int task_r, 
	int task_s, FileSet_t *fset)
{
	int count;
	int group = inWhichGroup (task_s, fset);

	if (-1 == group)
	{
		fprintf (stderr, "mpi2prv: Error! Invalid group for foreign receive. Dying...\n");
		fflush (stderr);
		exit (0);
	}
	count = ForeignRecvs[group].count;

	if (count == ForeignRecvs[group].size)
	{
		ForeignRecvs[group].size += FOREIGN_RECV_RESIZE_STEP;
		ForeignRecvs[group].data = (struct ForeignRecv_t*) 
			realloc (ForeignRecvs[group].data, 
					ForeignRecvs[group].size*sizeof(struct ForeignRecv_t));
	}
	ForeignRecvs[group].data[count].sender = task_s;
	ForeignRecvs[group].data[count].recver = task_r;
	ForeignRecvs[group].data[count].tag = tag;
	ForeignRecvs[group].data[count].physic = physic;
	ForeignRecvs[group].data[count].logic = logic;
	ForeignRecvs[group].count++;
}

void InitForeignRecvs (int numtasks)
{
	int i;

	ForeignRecvs = (struct ForeignRecvs_t *) malloc (sizeof(struct ForeignRecvs_t)*numtasks);

	for (i = 0; i < numtasks; i++)
	{
		ForeignRecvs[i].count = ForeignRecvs[i].size = 0;
		ForeignRecvs[i].data  = NULL;
	}
}

#if defined(OLD_DISTRIBUTION)
static void DistributeMyRecvs (int numtasks, int taskid)
{
	/* This function should match with ReceiveRecvsAndMatch in MPI terms*/
	int res, i;

	for (i = 0; i < numtasks; i++)
	{
		if (taskid != i)
		{
			if (ForeignRecvs[i].count > 0)
			{
				fprintf (stdout, "mpi2prv: Processor %d distributes %d foreign receives to processor %d\n", taskid, ForeignRecvs[i].count, i);
				fflush (stdout);
			}

			/* Send info on how many recvs will be transmitted */
			res = MPI_Send (&(ForeignRecvs[i].count), 1, MPI_INT, i, HOWMANY_FOREIGN_RECVS_TAG, MPI_COMM_WORLD);
			MPI_CHECK(res, MPI_Send, "Failed to send quantity of foreign receives");

			if (ForeignRecvs[i].count > 0)
			{
				/* Send data */
				res = MPI_Send (ForeignRecvs[i].data, ForeignRecvs[i].count*sizeof(struct ForeignRecv_t), MPI_BYTE, i, BUFFER_FOREIGN_RECVS_TAG, MPI_COMM_WORLD);
				MPI_CHECK(res, MPI_Send, "Failed to send foreign receives");

				/* Free data */
				free (ForeignRecvs[i].data);
			}
		}
	}
}
#else
static int DistributeMyRecvsTo (int taskid, int to, MPI_Request *r)
{
	int res;

	if (ForeignRecvs[to].count > 0)
	{
		fprintf (stdout, "mpi2prv: Processor %d distributes %d foreign receives to processor %d\n", taskid, ForeignRecvs[to].count, to);
		fflush (stdout);
	}

	/* Send info on how many recvs will be transmitted */
	res = MPI_Isend (&(ForeignRecvs[to].count), 1, MPI_INT, to, HOWMANY_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &(r[0]));
	MPI_CHECK(res, MPI_Isend, "Failed to send quantity of foreign receives");

	if (ForeignRecvs[to].count > 0)
	{
		/* Send data */
		res = MPI_Isend (ForeignRecvs[to].data, ForeignRecvs[to].count*sizeof(struct ForeignRecv_t), MPI_BYTE, to, BUFFER_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &(r[1]));
		MPI_CHECK(res, MPI_Isend, "Failed to send foreign receives");

		/* Free data */
		free (ForeignRecvs[to].data);
	}

	return (ForeignRecvs[to].count > 0)?2:1;
}
#endif

static void MatchRecv (int fd, off_t offset, UINT64 physic_time, UINT64 logic_time)
{
#if defined(DEAD_CODE)
	ssize_t size;
	paraver_rec_t r;
	off_t ret;

	/* Seek to find the information, gather all the record, change its
	   time and write back to the disk */
	ret = lseek (fd, offset, SEEK_SET);
	if (ret != offset)
	{
		perror ("lseek");
		fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to lseek (fd = %d, offset = %lld)\n", fd, offset);
		exit (-2);
	}
	size = read (fd, &r, sizeof(r));
	if (sizeof(r) != size)
	{
		perror ("read");
		fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to read (fd = %d, size = %d, read = %d)\n", fd, sizeof(r), size);
	}
	r.log_r = logic_time;
	r.phy_r = physic_time;
	ret = lseek (fd, offset, SEEK_SET);
	if (ret != offset)
	{
		perror ("lseek");
		fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to lseek (fd = %d, offset = %lld)\n", fd, offset);
		exit (-2);
	}
	size = write (fd, &r, sizeof(r));
	if (sizeof(r) != size)
	{
		perror ("write");
		fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to write (fd = %d, size = %d, written = %d)\n", fd, sizeof(r), size);
		exit (-2);
	}
#else
	paraver_rec_t r;
	ssize_t size;
	off_t ret;
	unsigned long long receives[NUM_COMMUNICATION_TYPE];
	long offset_in_struct;

  /* Search offset of receives within the paraver_rec_t struct */
  offset_in_struct = ((long) &r.receive) - ((long) &r);

  receives[LOGICAL_COMMUNICATION] = logic_time;
  receives[PHYSICAL_COMMUNICATION] = physic_time;

  ret = lseek (fd, offset+offset_in_struct, SEEK_SET);
  if (ret != offset)
  {
    perror ("lseek");
#if SIZEOF_OFF_T == SIZEOF_LONG
    fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to lseek (fd = %d, offset = %ld)\n", fd, offset);
#elif SIZEOF_OFF_T == SIZEOF_LONG_LONG
    fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to lseek (fd = %d, offset = %lld)\n", fd, offset);
#elif SIZEOF_OFF_T == 4
    fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to lseek (fd = %d, offset = %d)\n", fd, offset);
#endif
    exit (-2);
  }

  size = write (fd, &receives, sizeof(receives));
  if (sizeof(receives) != size)
  {
    perror ("write");
    fprintf (stderr, "mpi2prv: Error on MatchRecv! Unable to write (fd = %d, size = %d, written = %d)\n", fd, sizeof(r), size);
    exit (-2);
  }
#endif
}

static int MatchRecvs (struct ForeignRecv_t *data, int count)
{
	/* Match a set of "incomplete receives" */
	int i, j, result;

	result = 0;
	for (i = 0; i < count; i++)
		for (j = 0; j < PendingComms.count; j++)
		{
			if (data[i].sender == PendingComms.data[j].sender &&
			    data[i].recver == PendingComms.data[j].recver &&
			    data[i].tag == PendingComms.data[j].tag &&
			    !PendingComms.data[j].match)
			{
				PendingComms.data[j].match = 1;
				MatchRecv (PendingComms.data[j].descriptor, PendingComms.data[j].offset, data[i].physic, data[i].logic);
				result++;
				break;
			}
		}
	return result;
}

struct ForeignRecv_t* SearchForeignRecv (int group, int sender, int recver, int tag)
{
	int i;

	if (myForeignRecvs_count != NULL && myForeignRecvs != NULL)
	{
		if (myForeignRecvs[group] != NULL)
			for (i = 0; i < myForeignRecvs_count[group]; i++)
			{
				if (myForeignRecvs[group][i].sender == sender &&
				    myForeignRecvs[group][i].recver == recver &&
			  	  myForeignRecvs[group][i].tag == tag &&
			    	!myForeignRecvs_used[group][i])
				{
					myForeignRecvs_used[group][i] = TRUE;
					return &myForeignRecvs[group][i];
				}
			}
	}
	return NULL;
}

static int ReceiveAndMatchRecvs (int taskid, int numtasks, int source, int *total)
{
	/* This function should match with DistributeMyRecvs.
	   Receives a set of "incomplete receives" and matches them */
	MPI_Status s;
	int res, count;
	struct ForeignRecv_t *data;
	int match = 0;
	UNREFERENCED_PARAMETER(numtasks);

	res = MPI_Recv (&count, 1, MPI_INT, source, HOWMANY_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &s);
	MPI_CHECK(res, MPI_Recv, "Failed to receive count of foreign receives");

	if (count > 0)
	{
		data = (struct ForeignRecv_t*) malloc (count*sizeof(struct ForeignRecv_t));
		if (NULL == data)
		{
			fprintf (stderr, "mpi2prv: Error! Failed to allocate memory to receive foreign receives\n");
			fflush (stderr);
			exit (0);
		}
		res = MPI_Recv (data, count*sizeof(struct ForeignRecv_t), MPI_BYTE, source, BUFFER_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &s);
		MPI_CHECK(res, MPI_Recv, "Failed to receive foreign receives");

		match = MatchRecvs (data, count);

		free (data);
	}

	if (count > 0)
	{
		fprintf (stdout, "mpi2prv: Processor %d matched %d of %d communications from processor %d\n", taskid, match, count, source);
		fflush (stdout);
	}
	else
		fprintf (stdout, "mpi2prv: Processor %d did not receive foreign receives from processor %d\n", taskid, source);

	*total = count;
	return match;
}

void DistributePendingComms (int numtasks, int taskid)
{
	int res, i, match, total;

	if (0 == taskid)
	{
		fprintf (stdout, "mpi2prv: Starting the distribution of foreign receives.\n");
		fflush (stdout);
	}

#if defined(OLD_DISTRIBUTION)
	/* Distribute receives (and match them -- depending on who distributes
	   them --) among all the processors */
	for (i = 0; i < numtasks; i++)
	{
		if (taskid != i)
		{
			match = ReceiveAndMatchRecvs (numtasks, i, &total);
			if (total > 0)
			{
				fprintf (stdout, "mpi2prv: Processor %d matched %d of %d communications from processor %d\n", taskid, match, total, i);
				fflush (stdout);
			}
		}
		else
			DistributeMyRecvs (numtasks, taskid);

		res = MPI_Barrier (MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Barrier, "Failed to synchronize distribution of pending communications");
	}
#else
	for (i = 1; i < numtasks; i++)
	{
		int to, from, nsends;
		MPI_Request r[2];
		MPI_Status s[2];

		to = (taskid+i)%numtasks;
		from = (taskid+numtasks-i)%numtasks;

		nsends = DistributeMyRecvsTo (taskid, to, r);
		match = ReceiveAndMatchRecvs (taskid, numtasks, from, &total);

		res = MPI_Waitall (nsends, r, s);
		MPI_CHECK(res, MPI_Waitall, "Failed to wait sent pending communications");

		res = MPI_Barrier (MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Barrier, "Failed to synchronize distribution of pending communications");
	}
#endif

	if (0 == taskid)
	{
		fprintf (stdout, "mpi2prv: Ended the distribution of foreign receives.\n");
		fflush (stdout);
	}

	/* Free pending communication info */
	if (PendingComms.count > 0)
		free (PendingComms.data);
}

static int RecvMine (int taskid, int from, int match, int *out_count, struct ForeignRecv_t **out, char **used)
{
	MPI_Status s;
	int res, count;
	struct ForeignRecv_t *data;
	int num_match = 0;

	res = MPI_Recv (&count, 1, MPI_INT, from, HOWMANY_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &s);
	MPI_CHECK(res, MPI_Recv, "Failed to receive count of foreign receives");

	if (count > 0)
	{
		data = (struct ForeignRecv_t*) malloc (count*sizeof(struct ForeignRecv_t));
		if (NULL == data)
		{
			fprintf (stderr, "mpi2prv: Error! Failed to allocate memory to receive foreign receives\n");
			fflush (stderr);
			exit (0);
		}

		res = MPI_Recv (data, count*sizeof(struct ForeignRecv_t), MPI_BYTE, from, BUFFER_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, &s);
		MPI_CHECK(res, MPI_Recv, "Failed to receive foreign receives");
		if (match)
		{
			num_match = MatchRecvs (data, count);
			free (data);
		}
		else
		{
			int i;
			char *data_used;

			data_used = (char*) malloc (sizeof(char)*count);
			if (NULL == data_used)
			{
				fprintf (stderr, "mpi2prv: Error! Cannot create 'used' structure for foreign receives.\n");
				exit (-1);
			}
			for (i = 0; i < count; i++)
				data_used[i] = FALSE;

			*used = data_used;
			*out_count = count;
			*out = data;
		}
	}

	if (match)
	{
		if (count > 0)
			fprintf (stdout, "mpi2prv: Processor %d matched %d of %d communications from processor %d\n", taskid, num_match, count, from);
		else
			fprintf (stdout, "mpi2prv: Processor %d did not receive communications from processor %d\n", taskid, from);
	}
	
	fflush (stdout);

	return num_match;
}

static void SendMine (int taskid, int to, MPI_Request *req1, MPI_Request *req2)
{	
	int res;

	/* Send info on how many recvs will be transmitted */
	res = MPI_Isend (&(ForeignRecvs[to].count), 1, MPI_INT, to, HOWMANY_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, req1);
	MPI_CHECK(res, MPI_Isend, "Failed to send quantity of foreign receives");

	if (ForeignRecvs[to].count > 0)
	{
		fprintf (stdout, "mpi2prv: Processor %d distributes %d foreign receives to processor %d\n", taskid, ForeignRecvs[to].count, to);
		fflush (stdout);

		/* Send data */
		res = MPI_Isend (ForeignRecvs[to].data, ForeignRecvs[to].count*sizeof(struct ForeignRecv_t), MPI_BYTE, to, BUFFER_FOREIGN_RECVS_TAG, MPI_COMM_WORLD, req2);
		MPI_CHECK(res, MPI_Isend, "Failed to send foreign receives");
	}
	else
		fprintf(stdout, "mpi2prv: Processor %d does not have foreign receives for processor %d\n", taskid, to);
}


void NewDistributePendingComms (int numtasks, int taskid, int match)
{
	int i, skew, res;

	if (0 == taskid)
	{
		fprintf (stdout, "mpi2prv: Starting the distribution of foreign receives.\n");
		fflush (stdout);
	}

	if (!match)
	{
		myForeignRecvs = (struct ForeignRecv_t**) malloc (sizeof(struct ForeignRecv_t*)*numtasks);
		if (NULL == myForeignRecvs)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot allocate memory to control foreign receives!\n");
			exit (-1);
		}
		myForeignRecvs_used = (char**) malloc (sizeof(char*)*numtasks);
		if (NULL == myForeignRecvs_used)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot allocate memory to control foreign receives!\n");
			exit (-1);
		}
		myForeignRecvs_count = (int*) malloc (sizeof(int)*numtasks);
		if (NULL == myForeignRecvs_count)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot allocate memory to control the number of foreign receives!\n");
			exit (-1);
		}
		for (i = 0; i < numtasks; i++)
		{
			myForeignRecvs_count[i] = 0;
			myForeignRecvs[i] = NULL;
			myForeignRecvs_used[i] = NULL;
		}
	}

	for (skew = 1; skew < numtasks; skew++)
	{
		MPI_Request send_req1, send_req2;
		MPI_Status sts;
		int to, from;

		to = (taskid + skew) % numtasks;
		from = (taskid - skew + numtasks) % numtasks;

		SendMine (taskid, to, &send_req1, &send_req2);
		RecvMine (taskid, from, match, &myForeignRecvs_count[from], &myForeignRecvs[from], &myForeignRecvs_used[from]);

		MPI_Wait (&send_req1, &sts);
		if (ForeignRecvs[to].count > 0)
			MPI_Wait (&send_req2, &sts);

		/* Free data */
		free (ForeignRecvs[to].data);

/*
		res = MPI_Barrier (MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Barrier, "Failed to synchronize distribution of pending communications");
*/
	}

	res = MPI_Barrier (MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Barrier, "Failed to synchronize distribution of pending communications");

	if (!match)
	{
		int total;

		for (i = 0, total = 0; i < numtasks; i++)
			total += myForeignRecvs_count[i];

		fprintf (stdout, "mpi2prv: Processor %d is storing %d foreign receives (%lld Kbytes) for the next phase.\n",
			taskid, total, (((long long) total)*(sizeof(struct ForeignRecv_t)+sizeof(char)))/1024);
	}

	if (0 == taskid)
	{
		fprintf (stdout, "mpi2prv: Ended the distribution of foreign receives.\n");
		fflush (stdout);
	}

	/* Free pending communication info */
	if (PendingComms.count > 0)
		free (PendingComms.data);
}


static void BuildCommunicator (struct Communicator_t *new_comm)
{
	unsigned j;
	TipusComunicador com;

#if defined(DEBUG_COMMUNICATORS)
	fprintf (stdout, "mpi2prv: DEBUG Adding communicator type = %d ptask = %d task = %d\n", new_comm->type, new_comm->ptask, new_comm->task);
	if (new_comm->type != MPI_COMM_WORLD_ALIAS && new_comm->type != MPI_COMM_SELF_ALIAS)
	{
		fprintf (stdout, "mpi2prv: tasks:");
		for (j = 0; j < new_comm->ntasks; j++)
			fprintf (stdout, "%d \n", new_comm->tasks[j]);
		fprintf (stdout, "\n");
	}
#endif

	com.id = new_comm->id;
	com.num_tasks = new_comm->ntasks;
	com.tasks = (int*) malloc(sizeof(int)*com.num_tasks);
	if (NULL == com.tasks)
	{
		fprintf (stderr, "mpi2prv: Error! Unable to allocate memory for transferred communicator!\n");
		fflush (stderr);
		exit (-1);
	}

	if (MPI_COMM_WORLD_ALIAS == new_comm->type)
		for (j = 0; j < com.num_tasks; j++)
			com.tasks[j] = j;
	else if (MPI_COMM_SELF_ALIAS == new_comm->type)
		com.tasks[0] = new_comm->task-1;
	else
		for (j = 0; j < com.num_tasks; j++)
			com.tasks[j] = new_comm->tasks[j];

	afegir_comunicador (&com, new_comm->ptask, new_comm->task);

	free (com.tasks);
}

static void BroadCastCommunicator (int id, struct Communicator_t *new_comm)
{
	int res;

	res = MPI_Bcast (new_comm, sizeof(struct Communicator_t), MPI_BYTE, id, MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Bcast, "Failed to broadcast generated communicators");

	/* If comm isn't a predefined, send the involved tasks */
	if (MPI_COMM_SELF_ALIAS != new_comm->type && MPI_COMM_WORLD_ALIAS != new_comm->type)
	{
		res = MPI_Bcast (new_comm->tasks, new_comm->ntasks, MPI_INT, id, MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Bcast, "Failed to broadcast generated communicators");
	}
}

static void ReceiveCommunicator (int id)
{
	int res;
	struct Communicator_t tmp;

	res = MPI_Bcast (&tmp, sizeof(struct Communicator_t), MPI_BYTE, id, MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Bcast, "Failed to broadcast generated communicators");

	/* If comm isn't a predefined, receive the involved tasks */
	if (MPI_COMM_SELF_ALIAS != tmp.type && MPI_COMM_WORLD_ALIAS != tmp.type)
	{
		tmp.tasks = (int*) malloc (sizeof(int)*tmp.ntasks);
		if (NULL == tmp.tasks)
		{
			fprintf (stderr, "mpi2prv: ERROR! Failed to allocate memory for a new communicator body\n");
			fflush (stderr);
			exit (0);
		}
		res = MPI_Bcast (tmp.tasks, tmp.ntasks, MPI_INT, id, MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Bcast, "Failed to broadcast generated communicators");
	}
	BuildCommunicator (&tmp);

	/* Free data structures */
	if (tmp.tasks != NULL)
		free (tmp.tasks);
}

void BuildCommunicators (int num_tasks, int taskid)
{
	int i, j;
	int res, count;

	for (i = 0; i < num_tasks; i++)
	{
		if (i == taskid)
		{
			for (j = 0; j < Communicators.count; j++)
				BuildCommunicator (&(Communicators.comms[j]));

			res = MPI_Bcast (&Communicators.count, 1, MPI_INT, i, MPI_COMM_WORLD);
			MPI_CHECK(res, MPI_Bcast, "Failed to broadcast number of generated communicators");

			for (j = 0; j < Communicators.count; j++)
				BroadCastCommunicator (i, &(Communicators.comms[j]));

			/* Free data structures */
			for (j = 0; j < Communicators.count; j++)
				if (Communicators.comms[j].tasks != NULL)
					free (Communicators.comms[j].tasks);
			free (Communicators.comms);
		}
		else
		{
			res = MPI_Bcast (&count, 1, MPI_INT, i, MPI_COMM_WORLD);
			MPI_CHECK(res, MPI_Bcast, "Failed to broadcast number of generated communicators");
			for (j = 0; j < count; j++)
				ReceiveCommunicator (i);
		}
	}
}

void ShareTraceInformation (int numtasks, int taskid)
{
	int res;
	UNREFERENCED_PARAMETER(numtasks);

	res = MPI_Barrier (MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Bcast, "Failed to synchronize when sharing trace information");

	if (0 == taskid)
		fprintf (stdout, "mpi2prv: Sharing information <");
	fflush (stdout);

	if (0 == taskid)
		fprintf (stdout, " MPI");
	fflush (stdout);
	Share_MPI_Softcounter_Operations ();
	Share_MPI_Operations ();

	if (0 == taskid)
		fprintf (stdout, " OpenMP");
	fflush (stdout);
	Share_OMP_Operations ();

	if (0 == taskid)
		fprintf (stdout, " pthread");
	fflush (stdout);
	Share_pthread_Operations ();

#if USE_HARDWARE_COUNTERS
	if (0 == taskid)
		fprintf (stdout, " HWC");
	fflush (stdout);
	Share_Counters_Usage (numtasks, taskid);
#endif

	if (0 == taskid)
		fprintf (stdout, " MISC");
	fflush (stdout);
	Share_MISC_Operations ();

#if defined(HAVE_BFD)
	if (0 == taskid)
		fprintf (stdout, " callers");
	fflush (stdout);
	Share_Callers_Usage ();
#endif

	if (0 == taskid)
		fprintf (stdout, " >\n");
	fflush (stdout);
}

static void Receive_Dimemas_Data (void *buffer, int maxmem, int source, FILE *fd)
{
	ssize_t written;
	long long size;
	MPI_Status s;
	int res;
	int min;

	res = MPI_Recv (&size, 1, MPI_LONG_LONG, source, DIMEMAS_CHUNK_FILE_SIZE_TAG, MPI_COMM_WORLD, &s);
	MPI_CHECK(res, MPI_Recv, "Failed to receive file size of Dimemas chunk");

	do
	{
		min = MIN(maxmem, size);

		res = MPI_Recv (buffer, min, MPI_BYTE, source, DIMEMAS_CHUNK_DATA_TAG, MPI_COMM_WORLD, &s);
		MPI_CHECK(res, MPI_Recv, "Failed to receive file size of Dimemas chunk");

		written = write (fileno(fd), buffer, min);
		if (written != min)
		{
			perror ("write");
			fprintf (stderr, "mpi2trf: Error while writing the Dimemas trace file during parallel gather\n");
			fflush (stderr);
			exit (-1);
		}

		size -= min;
	} while (size > 0);
}

static void Send_Dimemas_Data (void *buffer, int maxmem, FILE *fd)
{
	ssize_t read_;
	long long size;
	int res;
	int min;

#if !defined(HAVE_FTELL64) && !defined(HAVE_FTELLO64)
	size = ftell (fd);
#elif defined(HAVE_FTELL64)
	size = ftell64 (fd);
#elif defined(HAVE_FTELLO64)
	size = ftello64 (fd);
#endif

	res = MPI_Send (&size, 1, MPI_LONG_LONG, 0, DIMEMAS_CHUNK_FILE_SIZE_TAG, MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Send, "Failed to send file size of Dimemas chunk");

	rewind (fd);
	fflush (fd);

	do
	{
		min = MIN(maxmem, size);
		read_ = read (fileno(fd), buffer, min);
		if (read_ != min)
		{
			perror ("read");
			fprintf (stderr, "mpi2trf: Error while reading the Dimemas trace file during parallel gather\n");
			fflush (stderr);
			exit (-1);
		}

		res = MPI_Send (buffer, min, MPI_BYTE, 0, DIMEMAS_CHUNK_DATA_TAG, MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Send, "Failed to receive file size of Dimemas chunk");

		size -= min;
	} while (size > 0);
}

void Gather_Dimemas_Traces (int numtasks, int taskid, FILE *fd, unsigned int maxmem)
{
	void *buffer;
	int res;
	int i;

	buffer = malloc (maxmem);
	if (NULL == buffer)
	{
		fprintf (stderr, "Error: mpi2trf was unable to allocate gathering buffers for Dimemas trace\n");
		fflush (stderr);
		exit (-1);
	}

	for (i = 1; i < numtasks; i++)
	{
		if (0 == taskid)
			Receive_Dimemas_Data (buffer, maxmem, i, fd);
		else if (i == taskid)
			Send_Dimemas_Data (buffer, maxmem, fd);

		res = MPI_Barrier (MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Barrier, "Failed to synchronize while gathering Dimemas trace");
	}

	free (buffer);
}


void Gather_Dimemas_Offsets (int numtasks, int taskid, int count,
	unsigned long long *in_offsets, unsigned long long **out_offsets,
	unsigned long long local_trace_size, FileSet_t *fset)
{
	unsigned long long other_trace_size;
	unsigned long long *temp = NULL;
	int res;
	int i;
	int j;

	if (0 == taskid)	
	{
		temp = (unsigned long long*) malloc (count*sizeof(unsigned long long));
		if (NULL == temp)
		{
			fprintf (stderr, "mpi2trf: Error! Unable to allocate memory for computing the offsets!\n");
			fflush (stderr);
			exit (-1);
		}
	}

	for (i = 0; i < numtasks-1; i++)
	{
		other_trace_size = (taskid == i)?local_trace_size:0;
		res = MPI_Bcast (&other_trace_size, 1, MPI_LONG_LONG, i, MPI_COMM_WORLD);
		MPI_CHECK(res, MPI_Bcast, "Failed to broadcast Dimemas local trace size");

		if (taskid > i)
			for (j = 0; j < count; j++)
				if (inWhichGroup (j, fset) == taskid)
					in_offsets[j] += other_trace_size;
	}

	res = MPI_Reduce (in_offsets, temp, count, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_CHECK(res, MPI_Reduce, "Failed to gather offsets for Dimemas trace");

	if (0 == taskid)
		*out_offsets = temp;
}

void ShareNodeNames (int numtasks, char ***nodenames)
{
	int i, rc;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int hostname_length;
	char *buffer_names = NULL;
	char **TasksNodes;

	/* Get processor name */
	rc = MPI_Get_processor_name (hostname, &hostname_length);
	MPI_CHECK(rc, MPI_Get_processor_name, "Cannot get processor name");

	/* Change spaces " " into underscores "_" (BGL nodes use to have spaces in their names) */
	for (i = 0; i < hostname_length; i++)
		if (' ' == hostname[i])
			hostname[i] = '_';

	/* Share information among all tasks */
	buffer_names = (char*) malloc (sizeof(char) * numtasks * MPI_MAX_PROCESSOR_NAME);
	rc = MPI_Allgather (hostname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, buffer_names, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, MPI_COMM_WORLD);
	MPI_CHECK(rc, MPI_Allgather, "Cannot gather processor names");

	/* Store the information in a global array */
	TasksNodes = (char **) malloc (numtasks * sizeof(char *));
	for (i = 0; i < numtasks; i++)
	{
		char *tmp = &buffer_names[i*MPI_MAX_PROCESSOR_NAME];
		TasksNodes[i] = (char *) malloc((strlen(tmp)+1) * sizeof(char));
		strcpy (TasksNodes[i], tmp);
	}

	/* Free the local array, not the global one */
	free (buffer_names);

	*nodenames = TasksNodes;
}

