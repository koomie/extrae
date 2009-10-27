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
 | @file: $Source: /home/paraver/cvs-tools/mpitrace/fusion/src/merger/paraver/write_file_buffer.c,v $
 | 
 | @last_commit: $Date: 2009/05/25 10:31:02 $
 | @version:     $Revision: 1.6 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "common.h"

static char UNUSED rcsid[] = "$Id: write_file_buffer.c,v 1.6 2009/05/25 10:31:02 harald Exp $";

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if HAVE_STRING_H
# include <string.h>
#endif
#if HAVE_STDIO_H
# include <stdio.h>
#endif
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "write_file_buffer.h"

WriteFileBuffer_t * WriteFileBuffer_new (char *filename, int maxElements, size_t sizeElement)
{
	WriteFileBuffer_t *res;

#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_new (%s, %d, %d)\n", filename, maxElements, sizeElement);
#endif

	res = (WriteFileBuffer_t*) malloc (sizeof(WriteFileBuffer_t));
	if (NULL == res)
	{
		fprintf (stderr, "mpi2prv: Cannot allocate WriteFileBuffer structure\n");
		exit (-1);
	}

	res->maxElements = maxElements;
	res->sizeElement = sizeElement;
	res->FD = open (filename, O_RDWR, 0600);
	if (-1 == res->FD)
	{
		fprintf (stderr, "mpi2prv: Cannot open file (%s) at WriteFileBuffer\n", filename);
		exit (-1);
	}
	res->numElements = 0;
	res->lastWrittenLocation = 0;
	res->Buffer = (void*) malloc (res->maxElements*sizeElement);
	if (NULL == res->Buffer)
	{
		fprintf (stderr, "mpi2prv: Cannot allocate memory for %d elements in WriteFileBuffer\n", maxElements);
		exit (-1);
	}

	return res;
}

void WriteFileBuffer_delete (WriteFileBuffer_t *wfb)
{
#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_delete (%p)\n", wfb);
#endif

	WriteFileBuffer_flush (wfb);
	if (-1 == close (wfb->FD))
	{
		fprintf (stderr, "mpi2prv: Error! Cannot close WriteFileBuffer!\n");
		exit (-1);
	}
	free (wfb->Buffer);
	free (wfb);
}

int WriteFileBuffer_getFD (WriteFileBuffer_t *wfb)
{
	return wfb->FD;
}

void WriteFileBuffer_flush (WriteFileBuffer_t *wfb)
{
	ssize_t res_write;

#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_flush (%p)\n", wfb);
#endif

	res_write = write (wfb->FD, wfb->Buffer, wfb->numElements*wfb->sizeElement);
	if (-1 == res_write)
	{
		fprintf (stderr, "mpi2prv: Error! Cannot write WriteFileBuffer for flushing!\n");
		exit (-1);
	}
	else if (wfb->numElements*wfb->sizeElement != res_write)
	{
		fprintf (stderr, "mpi2prv: Error! Could not write %d bytes to disk\n"
		                 "mpi2prv: Error! Check your quota or set TMPDIR to a free disk zone\n", wfb->numElements*wfb->sizeElement);
		exit (-1);
	}

	wfb->lastWrittenLocation = lseek (wfb->FD, 0, SEEK_END);
	if (-1 == wfb->lastWrittenLocation)
	{
		fprintf (stderr, "mpi2prv: Error! Cannot retrieve last written location for WriteFileBuffer\n");
		exit (-1);
	}
	wfb->numElements = 0;
}

off_t WriteFileBuffer_getPosition (WriteFileBuffer_t *wfb)
{
#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_getPosition (%p)\n", wfb);
#endif
	return wfb->lastWrittenLocation+wfb->numElements*wfb->sizeElement;
}

void WriteFileBuffer_write (WriteFileBuffer_t *wfb, const void* data)
{
	size_t offset;

#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_write (%p, %p)\n", wfb, data);
#endif

	offset = wfb->numElements*wfb->sizeElement;
	memcpy ((((char*)wfb->Buffer)+offset), data, wfb->sizeElement);
	wfb->numElements++;

	if (wfb->numElements == wfb->maxElements)
		WriteFileBuffer_flush (wfb);
}

void WriteFileBuffer_writeAt (WriteFileBuffer_t *wfb, const void* data, off_t position)
{
#if defined(DEBUG)
	fprintf (stderr, "WriteFileBuffer_writeAt (%p, %p, %ld)\n", wfb, data, position);
#endif

	if (position < wfb->lastWrittenLocation)
	{
		/* this is outside and before the buffer */
		off_t lseek_res;
		ssize_t write_res;

		lseek_res = lseek (wfb->FD, position, SEEK_SET);
		if (-1 == lseek_res)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot lseek when performing WriteFileBuffer_writeAt\n");
			exit (-1);
		}
		write_res = write (wfb->FD, data, wfb->sizeElement);
		if (-1 == write_res)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot write when performing write_WriteFileBufferAt\n");
			exit (-1);
		}
		lseek_res = lseek (wfb->FD, wfb->lastWrittenLocation, SEEK_SET);
		if (-1 == lseek_res)
		{
			fprintf (stderr, "mpi2prv: Error! Cannot lseek after performing write_WriteFileBufferAt\n");
			exit (-1);
		}
	}
	else
	{
		if (position+wfb->sizeElement >
		    wfb->lastWrittenLocation+wfb->sizeElement*wfb->numElements)
		{
			/* the write is beyond the limit of the file, abort it */
			fprintf (stderr, "mpi2prv: Error! Cannot perform WriteFileBuffer_writeAt. Given position is out ouf bounds.\n");
			fprintf (stderr, "mpi2prv: Position = %ld, limit = %ld (numelements = %d)\n",
			         position+wfb->sizeElement,
			         wfb->lastWrittenLocation+wfb->sizeElement*wfb->numElements,
			         wfb->numElements);
			exit (-1);
		}
		else
		{
			/* the write is inside the limit of the file, AND, inside the buffer */
			size_t offset = position - wfb->lastWrittenLocation;
			memcpy ((((char*)wfb->Buffer)+offset), data, wfb->sizeElement);
		}
	}
}

void WriteFileBuffer_removeLast (WriteFileBuffer_t *wfb)
{
#if defined(DEBUG)
  fprintf (stderr, "WriteFileBuffer_removeLast (%p)\n", wfb);
#endif

	if (wfb->numElements > 0)
	{
		/* if exists in the buffer, just remove its accounting */
		wfb->numElements--;
	}
	else if (wfb->numElements == 0)
	{
		/* The buffer was just flushed. Now if it has contents, truncate it */
		if (wfb->lastWrittenLocation >= wfb->sizeElement)
		{
			int res = ftruncate (wfb->FD, wfb->lastWrittenLocation-wfb->sizeElement);
			if (-1 == res)
			{
				fprintf (stderr, "mpi2prv: Error! Could not truncate the file pointed by the WriteFileBuffer\n");
				exit (-1);
			}
		}
	}
}

