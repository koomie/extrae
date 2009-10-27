/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
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
 | @file: $Source: /home/paraver/cvs-tools/mpitrace/fusion/src/launcher/dyninst/mini-xml-parse.h,v $
 | 
 | @last_commit: $Date: 2009/01/07 16:44:13 $
 | @version:     $Revision: 1.1 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _XML_PARSE_H_INCLUDED_
#define _XML_PARSE_H_INCLUDED_

#define TRACE_TAG                       ((xmlChar*) "trace")
#define TRACE_HOME                      ((xmlChar*) "home")
#define TRACE_TYPE                      ((xmlChar*) "type")
#define TRACE_PARSER_ID                 ((xmlChar*) "xml-parser-id")
#define TRACE_TYPE_PARAVER              ((xmlChar*) "paraver")
#define TRACE_TYPE_DIMEMAS              ((xmlChar*) "dimemas")
#define TRACE_INITIAL_MODE              ((xmlChar*) "initial-mode")
#define TRACE_INITIAL_MODE_DETAIL       ((xmlChar*) "detail")
#define TRACE_INITIAL_MODE_BURSTS       ((xmlChar*) "bursts")
#define TRACE_COUNTERS                  ((xmlChar*) "counters")
#define TRACE_CALLERS                   ((xmlChar*) "callers")
#define TRACE_CPU                       ((xmlChar*) "cpu")
#define TRACE_STARTSET                  ((xmlChar*) "starting-set-distribution")
#define TRACE_HWCSET                    ((xmlChar*) "set")
#define TRACE_HWCSET_CHANGEAT_GLOBALOPS ((xmlChar*) "changeat-globalops")
#define TRACE_HWCSET_CHANGEAT_TIME      "changeat-time"
#define TRACE_HWCSET_DOMAIN             ((xmlChar*) "domain")
#define TRACE_HWCSET_OVERFLOW_COUNTER   ((xmlChar*) "overflow-counter")
#define TRACE_HWCSET_OVERFLOW_VALUE     ((xmlChar*) "overflow-value")
#define TRACE_ENABLED                   ((xmlChar*) "enabled")
#define TRACE_SIZE                      ((xmlChar*) "size")
#define TRACE_MPI_CALLERS               ((xmlChar*) "callers")
#define TRACE_FINAL_DIR                 ((xmlChar*) "final-directory")
#define TRACE_DIR                       ((xmlChar*) "temporal-directory")
#define TRACE_MKDIR                     ((xmlChar*) "make-dir")
#define TRACE_MINIMUM_TIME              ((xmlChar*) "minimum-time")
#define TRACE_FREQUENCY                 ((xmlChar*) "frequency")
#define TRACE_GATHER_MPITS              ((xmlChar*) "gather-mpits")
#define TRACE_CIRCULAR                  ((xmlChar*) "circular")
#define TRACE_PREFIX                    ((xmlChar*) "trace-prefix")
#define TRACE_MPI                       ((xmlChar*) "mpi")
#define TRACE_OMP_LOCKS                 ((xmlChar*) "locks")
#define TRACE_OMP                       ((xmlChar*) "openmp")
#define TRACE_STORAGE                   ((xmlChar*) "storage")
#define TRACE_BUFFER                    ((xmlChar*) "buffer")
#define TRACE_OTHERS                    ((xmlChar*) "others")
#define TRACE_BURSTS                    ((xmlChar*) "bursts")
#define TRACE_THRESHOLD                 ((xmlChar*) "threshold")
#define TRACE_NETWORK                   ((xmlChar*) "network")
#define TRACE_MPI_STATISTICS            ((xmlChar*) "mpi-statistics")
#define TRACE_RUSAGE                    ((xmlChar*) "resource-usage")
#define TRACE_CELL                      ((xmlChar*) "cell")
#define TRACE_SPU_DMATAG                ((xmlChar*) "spu-dma-channel")
#define TRACE_SPU_FILESIZE              ((xmlChar*) "spu-file-size")
#define TRACE_SPU_BUFFERSIZE            ((xmlChar*) "spu-buffer-size")
#define TRACE_LIST                      ((xmlChar*) "list")
#define TRACE_USERFUNCTION              ((xmlChar*) "user-functions")
#define TRACE_MAX_DEPTH                 ((xmlChar*) "max-depth")
#define TRACE_SAMPLING                  ((xmlChar*) "sampling")
#define TRACE_FREQUENCY                 ((xmlChar*) "frequency")

#define TRACE_CONTROL                   ((xmlChar*) "trace-control")
#define TRACE_CONTROL_FILE              ((xmlChar*) "file")
#define TRACE_CONTROL_GLOPS             ((xmlChar*) "global-ops")
#define TRACE_REMOTE_CONTROL            ((xmlChar*) "remote-control")
#define TRACE_REMOTE_CONTROL_METHOD     ((xmlChar*) "method")
#define TRACE_REMOTE_CONTROL_MRNET      ((xmlChar*) "mrnet")
#define TRACE_REMOTE_CONTROL_SIGNAL     ((xmlChar*) "signal")

#ifdef  __cplusplus
extern "C" {
#endif /* c++ */

  void Parse_XML_File (int, int, char*);

  int XML_CheckTraceEnabled (void);

  char * XML_GetFinalDirectory (void);

  char * XML_GetTracePrefix (void);

	int XML_GetTraceMPI (void);

	int XML_GetTraceOMP (void);

	int XML_GetTraceOMP_locks (void);

	char * XML_UFlist (void);

	int XML_have_UFlist (void);


#ifdef  __cplusplus
}
#endif /* c++ */

#endif
