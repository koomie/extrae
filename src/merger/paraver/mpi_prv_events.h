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
 | @file: $Source: /home/paraver/cvs-tools/mpitrace/fusion/src/merger/paraver/mpi_prv_events.h,v $
 | 
 | @last_commit: $Date: 2008/01/23 15:00:00 $
 | @version:     $Revision: 1.2 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef MPI_PRV_EVENTS_H
#define MPI_PRV_EVENTS_H

#include <config.h>

#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include "common.h"

/*************************************************************************
 * S'utilitza el format nou dels .prv, que genera diferents tipus i estan
 * definits en un fitxer de constants.
 *************************************************************************/

void Enable_Soft_Counter(unsigned int EvType);

/* S'afegeix el fitxer on hi ha totes les constants */
#include "MPI_EventEncoding.h"

#define MPITYPE_FLAG_COLOR 9

#define NUM_BLOCK_GROUPS  8     /* Dels 12, de moment nomes 8 son diferents */


struct t_event_mpit2prv
{
  int tipus_mpit;
  int tipus_prv;
  int valor_prv;
  int utilitzada;               /* Boolea que indica si apareix a la trac,a */
};

struct t_prv_type_info
{
  int type;
  char *label;
  int flag_color;
};

struct t_prv_val_label
{
  int value;
  char *label;
};


#define NUM_PRV_ELEMENTS 150    /* 127 */

extern struct t_event_mpit2prv event_mpit2prv[];
extern struct t_prv_type_info prv_block_groups[];
void MPISoftCountersEvent_WriteEnabledOperations(FILE *fd);
int busca_event_mpit (int tmpit);
void MPITEvent_WriteEnabledOperations (FILE * fd);
void Enable_MPI_Operation (int tmpit);

#if defined(PARALLEL_MERGE)
void Share_MPI_Softcounter_Operations (void);
void Share_MPI_Operations (void);
#endif

void Translate_MPI_MPIT2PRV (int typempit, UINT64 valuempit, int *typeprv, UINT64 *valueprv);

/* MACRO per obtenir facilment un tipus de block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_TYPE(i)  prv_block_groups[i].type
/* MACRO per obtenir facilment una etiqueta de block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_LABEL(i) prv_block_groups[i].label
/* MACRO per obtenir facilment el color d'un block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_COLOR(i) prv_block_groups[i].flag_color


#endif
