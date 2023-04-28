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
	${OBJECTDIR}/service_monitor_main.o


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
LDLIBSOPTIONS=-L/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libutils1 -L/home/pj/NetBeansProjects/proto1 -Wl,-rpath,/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -Wl,-rpath,/home/pj/NetBeansProjects/libutils1 -Wl,-rpath,/home/pj/NetBeansProjects/proto1 `pkg-config --libs libxml-2.0` `pkg-config --libs libcares` `pkg-config --libs jansson` `pkg-config --libs uuid` `pkg-config --libs libuv` `pkg-config --libs protobuf` -Wl,-rpath,/home/pj/NetBeansProjects/proto1/protocpp -L/home/pj/NetBeansProjects/proto1/protocpp -lpblib -Wl,-rpath,/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -llibutils1 -Wl,-rpath,/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -llibcmn2 -Wl,-rpath,/home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86 -lappcmn1  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor: /home/pj/NetBeansProjects/proto1/protocpp/libpblib.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor: /home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86/liblibutils1.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor: /home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86/liblibcmn2.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor: /home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86/libappcmn1.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/service_monitor_main.o: service_monitor_main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/libutils1 -I/home/pj/NetBeansProjects/appcmn1 -I/home/pj/NetBeansProjects/proto1/protocpp `pkg-config --cflags libxml-2.0` `pkg-config --cflags libcares` `pkg-config --cflags jansson` `pkg-config --cflags uuid` `pkg-config --cflags libuv` `pkg-config --cflags protobuf`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service_monitor_main.o service_monitor_main.cpp

# Subprojects
.build-subprojects:
	cd /home/pj/NetBeansProjects/appcmn1 && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service_monitor

# Subprojects
.clean-subprojects:
	cd /home/pj/NetBeansProjects/appcmn1 && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
