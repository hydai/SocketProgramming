CC=g++
CLI=HW3_101062124_Cli
SER=HW3_101062124_Ser
LIB=hw3_*.cpp
RTYPE=cpp
CXXFLAG=-std=c++11
LINKFLAG=-lsqlite3
all:
	${CC} ${SER}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${SER}.out -lpthread
	${CC} ${CLI}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${CLI}.out -lpthread

clean:
	rm *.out
