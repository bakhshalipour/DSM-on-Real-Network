#################################################
##
## Copyright © 2009-2014 Behnam Momeni.
## All rights reserved.
##
## This file is part of the Partov project.
##
#################################################

CXX = g++
FLAGS = -Wall -g -Ibase
LIBS = -lpthread

TARGET_ROUTER = router.out
TARGET_PROCESSOR = processor.out
TARGET = ${TARGET_ROUTER} ${TARGET_PROCESSOR}

BASE_SOURCES = base/main.cpp \
	base/cf.cpp \
	base/interface.cpp \
	base/machine.cpp \
	base/frame.cpp

SUB_NAME = PA1

# DO NOT LIST .H FILES HERE! JUST .C AND .CPP FILES
USER_ROUTER_SOURCES = user-router/sm.cpp \
	user-router/facade.cpp

USER_PROCESSOR_SOURCES = user-processor/sm.cpp \
	user-processor/facade.cpp

BASE_OBJ = ${BASE_SOURCES:.cpp=.o}
BASE_OBJECTS = ${BASE_OBJ:.c=.o}

USER_ROUTER_OBJ = ${USER_ROUTER_SOURCES:.cpp=.o}
USER_ROUTER_OBJECTS = ${USER_ROUTER_OBJ:.c=.o}
USER_PROCESSOR_OBJ = ${USER_PROCESSOR_SOURCES:.cpp=.o}
USER_PROCESSOR_OBJECTS = ${USER_PROCESSOR_OBJ:.c=.o}

OBJECTS_ROUTER = ${BASE_OBJECTS} ${USER_ROUTER_OBJECTS}
OBJECTS_PROCESSOR = ${BASE_OBJECTS} ${USER_PROCESSOR_OBJECTS}

OBJECTS = ${BASE_OBJECTS} ${USER_ROUTER_OBJECTS} ${USER_PROCESSOR_OBJECTS}

all: ${TARGET_ROUTER} ${TARGET_PROCESSOR}

${TARGET_ROUTER}: ${OBJECTS_ROUTER}
	${CXX} ${FLAGS} -o ${TARGET_ROUTER} ${OBJECTS_ROUTER} ${LIBS}

${TARGET_PROCESSOR}: ${OBJECTS_PROCESSOR}
	${CXX} ${FLAGS} -o ${TARGET_PROCESSOR} ${OBJECTS_PROCESSOR} ${LIBS}

clean:
	tmpfolder=`echo /tmp/CF-clean-$$$$.removed` && mkdir -p $$tmpfolder && date >a && files="a `for a in ${OBJECTS} ${TARGET} *~ */*~; do if [ -e $$a ]; then echo $$a; fi; done`" && tar cf $$tmpfolder/backup.tar $$files && rm -f $$files

submit:
	zip -r ${SUB_NAME}_`date +%s`.zip user*/*.c user*/*.cpp user*/*.h Makefile

.cpp.o:
	${CXX} ${FLAGS} -c $< -o $@
.c.o:
	${CXX} ${FLAGS} -c $< -o $@

