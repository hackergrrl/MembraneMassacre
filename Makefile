OBJS = enemy.o main.o map.o menu.o player.o sprites.o utils.o weapons.o
SRCS = ${OBJS:.o=.cpp}
DEFINES =

UNAME := $(shell uname)
ifeq (${UNAME}, Darwin)
LIBS := -framework OpenGL
INCDIR := -I/opt/local/include
DEFINES := -DMAC_OS_X
endif
ifeq (${UNAME}, Linux)
#...
DEFINES := -DLINUX
endif
ifeq (${UNAME}, SunOS)
#...
DEFINES := -DSOLARIS
endif

INCDIR := ${INCDIR} `freetype-config --cflags`
LIBS := ${LIBS} `allegro-config --libs` -lfreetype -lXrandr

COMPILER = g++
COMPILER_FLAGS = -ggdb ${DEFINES}
TARGET = game
DEPEND = depend.mk

.SUFFIXES: .cpp .o

.cpp.o:
	${COMPILER} -c ${INCDIR} ${COMPILER_FLAGS} $<

game: ${OBJS}
	${COMPILER} -o ${TARGET} ${LIBDIR} ${LIBS} ${OBJS}

all: ${TARGET}

depend:
	${COMPILER} ${INCDIR} -MM ${SRCS} > ${DEPEND}

clean:
	rm -f ${OBJS} ${TARGET} *~

run:
	./${TARGET}

-include ${DEPEND}

