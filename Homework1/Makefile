CC=g++
CLI=HW1_101062124_Cli
SER=HW1_101062124_Ser
RTYPE=cpp
all:
	${CC} ${SER}.${RTYPE} -o ${SER}.out
	${CC} ${CLI}.${RTYPE} -o ${CLI}.out

debug:
	${CC} ${SER}.${RTYPE} -o ${SER}.out -g
	${CC} ${CLI}.${RTYPE} -o ${CLI}.out -g

clean:
	rm *.out
