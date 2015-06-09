CC=g++
CLI=hw3_101062124_Cli
SER=hw3_101062124_Ser
LIB=hw3_*.cpp
RTYPE=cpp
CXXFLAG=-std=c++11
LINKFLAG=-lsqlite3
all:
	${CC} ${SER}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${SER}.out
	${CC} ${CLI}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${CLI}.out

clean:
	rm *.out
