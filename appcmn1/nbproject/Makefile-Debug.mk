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
	${OBJECTDIR}/appcmn1.o \
	${OBJECTDIR}/pbcommon.o \
	${OBJECTDIR}/session.o


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
LDLIBSOPTIONS=`pkg-config --libs libuv` `pkg-config --libs protobuf` -Wl,-rpath,/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -llibutils1 -Wl,-rpath,/home/pj/NetBeansProjects/proto1/protocpp -L/home/pj/NetBeansProjects/proto1/protocpp -lpblib  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT}: /home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86/liblibutils1.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT}: /home/pj/NetBeansProjects/proto1/protocpp/libpblib.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	gcc -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/appcmn1.o: appcmn1.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libutils1 -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/proto1/protocpp `pkg-config --cflags libuv` `pkg-config --cflags protobuf`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/appcmn1.o appcmn1.cpp

${OBJECTDIR}/pbcommon.o: pbcommon.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libutils1 -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/proto1/protocpp `pkg-config --cflags libuv` `pkg-config --cflags protobuf`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pbcommon.o pbcommon.cpp

${OBJECTDIR}/session.o: session.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libutils1 -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/proto1/protocpp `pkg-config --cflags libuv` `pkg-config --cflags protobuf`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/session.o session.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libappcmn1.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
