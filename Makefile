OBJS = enemy.o main.o map.o menu.o player.o sprites.o utils.o weapons.o
SRCS = ${OBJS:.o=.cpp}
DEFINES =

COMPILER = g++
COMPILER_FLAGS = -ggdb ${DEFINES}
TARGET = game
DEPEND = depend.mk

UNAME := $(shell uname)
ifeq (${UNAME}, Darwin)
LIBS := -framework OpenGL
INCDIR := -I/opt/local/include
DEFINES := -DMAC_OS_X
COMPILER_FLAGS := ${COMPILER_FLAGS} -arch i386
endif
ifeq (${UNAME}, Linux)
#...
DEFINES := -DLINUX
endif
ifeq (${UNAME}, SunOS)
#...
DEFINES := -DSOLARIS
endif

INCDIR := ${INCDIR} `allegro-config --cflags`
LIBS := ${LIBS} `allegro-config --static`

.SUFFIXES: .cpp .o

.cpp.o:
	${COMPILER} -c ${INCDIR} ${COMPILER_FLAGS} $<

game: ${OBJS}
	${COMPILER} -o ${TARGET} -arch i386 ${LIBDIR} ${LIBS} ${OBJS}

all: ${TARGET}

depend:
	${COMPILER} ${INCDIR} -MM ${SRCS} > ${DEPEND}

clean:
	rm -f ${OBJS} ${TARGET} *~

run:
	./${TARGET}

-include ${DEPEND}

