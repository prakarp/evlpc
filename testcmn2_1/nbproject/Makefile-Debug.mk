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
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/home/pj/NetBeansProjects/proto1/protocpp -Wl,-rpath,/home/pj/NetBeansProjects/proto1/protocpp -Wl,-rpath,/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -llibcmn2 -Wl,-rpath,/home/pj/NetBeansProjects/proto1/protocpp -L/home/pj/NetBeansProjects/proto1/protocpp -lpblib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1: /home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86/liblibcmn2.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1: /home/pj/NetBeansProjects/proto1/protocpp/pblib.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/proto1/protocpp -I/home/pj/NetBeansProjects/libutils1 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:
	cd /home/pj/NetBeansProjects/libcmn2 && ${MAKE}  -f Makefile CONF=Debug
	cd /home/pj/NetBeansProjects/proto1 && ${MAKE} -f Makefile

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testcmn2_1

# Subprojects
.clean-subprojects:
	cd /home/pj/NetBeansProjects/libcmn2 && ${MAKE}  -f Makefile CONF=Debug clean
	cd /home/pj/NetBeansProjects/proto1 && ${MAKE} -f Makefile clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
