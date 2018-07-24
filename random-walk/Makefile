EXECS=random_walk
MPICXX?=mpicxx

all: ${EXECS}

random_walk: random-walk.cc
	${MPICXX} -o random-walk random-walk.cc

clean:
	rm -f ${EXECS}
