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
	${OBJECTDIR}/cfgagent_main.o


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
LDLIBSOPTIONS=-Wl,-rpath,/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86 -llibcmn2 -Wl,-rpath,/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86 -llibutils1 `pkg-config --libs libuv` -Wl,-rpath,/home/pj/NetBeansProjects/proto1/protocpp -L/home/pj/NetBeansProjects/proto1/protocpp -lpblib `pkg-config --libs jansson` `pkg-config --libs protobuf` -Wl,-rpath,/home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86 -L/home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86 -lappcmn1  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent: /home/pj/NetBeansProjects/libcmn2/dist/Debug/GNU-Linux-x86/liblibcmn2.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent: /home/pj/NetBeansProjects/libutils1/dist/Debug/GNU-Linux-x86/liblibutils1.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent: /home/pj/NetBeansProjects/proto1/protocpp/libpblib.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent: /home/pj/NetBeansProjects/appcmn1/dist/Debug/GNU-Linux-x86/libappcmn1.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/cfgagent_main.o: cfgagent_main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/pj/NetBeansProjects/libcmn2 -I/home/pj/NetBeansProjects/libutils1 -I/home/pj/NetBeansProjects/proto1/protocpp -I/home/pj/NetBeansProjects/appcmn1 `pkg-config --cflags libuv` `pkg-config --cflags jansson` `pkg-config --cflags protobuf`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cfgagent_main.o cfgagent_main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cfgagent

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
