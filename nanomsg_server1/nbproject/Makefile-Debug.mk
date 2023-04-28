#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-pthread

# CC Compiler Flags
CCFLAGS=-pthread
CXXFLAGS=-pthread

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/home/pj/projects/local/lib -L/home/pj/NetBeansProjects/libcommon/dist/Debug/GNU-Linux-x86 -Wl,-rpath,/home/pj/projects/local/lib -Wl,-rpath,/home/pj/NetBeansProjects/libcommon/dist/Debug/GNU-Linux-x86 -lnanomsg -Wl,-rpath,/home/pj/NetBeansProjects/libcommon/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libcommon/dist/Debug/GNU-Linux-x86 -llibcommon

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nanomsg_server1

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nanomsg_server1: /home/pj/NetBeansProjects/libcommon/dist/Debug/GNU-Linux-x86/liblibcommon.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nanomsg_server1: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nanomsg_server1 ${OBJECTFILES} ${LDLIBSOPTIONS} -pthread

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/projects/local/include -I/home/pj/NetBeansProjects/libcommon -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:
	cd /home/pj/NetBeansProjects/libcommon && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nanomsg_server1

# Subprojects
.clean-subprojects:
	cd /home/pj/NetBeansProjects/libcommon && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
