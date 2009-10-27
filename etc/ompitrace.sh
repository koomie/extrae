#
# This file is automatically generated by the MPItrace instrumentation Makefile.
# Edit with caution
#

if test "${MPITRACE_HOME}" != "" ; then

	# Read configuration variables if available!
	if ! test -f ${MPITRACE_HOME}/etc/ompitrace-vars.sh ; then
		echo "Error! Unable to locate ${MPITRACE_HOME}/etc/ompitrace-vars.sh"
		echo "Dying..."
		break
	else
		source ${MPITRACE_HOME}/etc/ompitrace-vars.sh
	fi

	export LD_LIBRARY_PATH=${MPITRACE_HOME}/lib:${LD_LIBRARY_PATH}

	if test "${MPI_HOME}" != "" ; then
		if ! test -d ${MPI_HOME}/lib ; then
			echo "Unable to find libmpi library directory!"
		else
			export LD_LIBRARY_PATH=${MPI_HOME}/lib:${LD_LIBRARY_PATH}
		fi
	fi

	if test "${LIBXML2_HOME}" != "" ; then
		if ! test -d ${LIBXML2_HOME}/lib ; then
			echo "Unable to find libxml2 library directory!"
		else
			export LD_LIBRARY_PATH=${LIBXML2_HOME}/lib:${LD_LIBRARY_PATH}
		fi
	fi

	if test "${PAPI_HOME}" != "" ; then
		if ! test -d ${PAPI_HOME}/lib ; then
			echo "Unable to find PAPI library directory!"
		else
			export LD_LIBRARY_PATH=${PAPI_HOME}/lib:${LD_LIBRARY_PATH}
		fi
	fi

	if test "${DYNINST_HOME}" != "" ; then
		if ! test -d ${DYNINST_HOME}/lib ; then
			echo "Unable to find DynInst library directory!"
		else
			if ! test -f ${DYNINST_HOME}/lib/libdyninstAPI_RT.so.1 ; then
				echo "Unable to find libdyninstAPI_RT.so.1 in the Dyninst library directory!"
			else
				export LD_LIBRARY_PATH=${DYNINST_HOME}/lib:${LD_LIBRARY_PATH}
				export DYNINSTAPI_RT_LIB=${DYNINST_HOME}/lib/libdyninstAPI_RT.so.1
			fi
		fi
	fi

else
	echo "You have to define MPITRACE_HOME to run this script"
fi

