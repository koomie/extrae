#
# This file is automatically generated by the MPItrace instrumentation Makefile.
# Edit with caution
#

if (${?MPITRACE_HOME}) then

	# Read configuration variables (if available)
	if (! -f ${MPITRACE_HOME}/etc/ompitrace-vars.csh) then
		echo "Error! Unable to locate ${MPITRACE_HOME}/etc/ompitrace-vars.sh"
		echo "Dying..."
		break
	else
		source ${MPITRACE_HOME}/etc/ompitrace-vars.csh
	endif

	if (${?LD_LIBRARY_PATH}) then
		setenv LD_LIBRARY_PATH ${MPITRACE_HOME}/lib:${LD_LIBRARY_PATH}
	else
		setenv LD_LIBRARY_PATH ${MPITRACE_HOME}/lib
	endif

	if (${?MPI_HOME}) then
		if (! -d ${MPI_HOME}/lib ) then
			echo "Unable to find libmpi library directory!"
		else
			setenv LD_LIBRARY_PATH ${MPI_HOME}/lib:${LD_LIBRARY_PATH}
		endif
	endif

	if (${?LIBXML2_HOME}) then
		if (! -d ${LIBXML2_HOME}/lib ) then
			echo "Unable to find libxml2 library directory!"
		else
			setenv LD_LIBRARY_PATH ${LIBXML2_HOME}/lib:${LD_LIBRARY_PATH}
		endif
	endif

	if (${?PAPI_HOME}) then
		if (! -d ${PAPI_HOME}/lib ) then
			echo "Unable to find PAPI library directory!"
		else
			setenv LD_LIBRARY_PATH ${PAPI_HOME}/lib:${LD_LIBRARY_PATH}
		endif
	endif

	if (${?DYNINST_HOME}) then
		if (! -d ${DYNINST_HOME}/lib ) then
			echo "Unable to find DynInst library directory!"
		else
			if (! -f ${DYNINST_HOME}/lib/libdyninstAPI_RT.so.1 ) then
				echo "Unable to find libdyninstAPI_RT.so.1 in the Dyninst library directory!"
			else
				setenv LD_LIBRARY_PATH ${DYNINST_HOME}/lib:${LD_LIBRARY_PATH}
				setenv DYNINSTAPI_RT_LIB ${DYNINST_HOME}/lib/libdyninstAPI_RT.so.1
			endif
		endif
	endif

else
	echo "You have to define MPITRACE_HOME to run this script"
endif

