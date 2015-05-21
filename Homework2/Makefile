CC=g++
CLI=HW2_101062124_Cli
SER=HW2_101062124_Ser
LIB=hw2_lib
RTYPE=cpp
CXXFLAG=-std=c++11
LINKFLAG=-lsqlite3
all:
	${CC} ${SER}.${RTYPE} ${LIB}.${RTYPE} ${CXXFLAG} ${LINKFLAG} -o ${SER}.out
	${CC} ${CLI}.${RTYPE} ${LIB}.${RTYPE} ${CXXFLAG} ${LINKFLAG} -o ${CLI}.out

clean:
	rm *.out
