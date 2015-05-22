CC=g++
CLI=HW2_101062124_Cli
SER=HW2_101062124_Ser
LIB=hw2_*.cpp
RTYPE=cpp
CXXFLAG=-std=c++11
LINKFLAG=-lsqlite3
all:
	${CC} ${SER}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${SER}.out
	${CC} ${CLI}.${RTYPE} ${LIB} ${CXXFLAG} ${LINKFLAG} -o ${CLI}.out

clean:
	rm *.out
