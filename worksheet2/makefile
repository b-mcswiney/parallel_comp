
CC = mpicc
CCFLAGS = -Wall -lm -std=c99

all:
	$(CC) $(CCFLAGS) -o vectorAddition vectorAddition.c
	$(CC) $(CCFLAGS) -o vectorDouble vectorDouble.c
	$(CC) $(CCFLAGS) -o cyclicSendAndReceive cyclicSendAndReceive.c
	$(CC) $(CCFLAGS) -o distributedCount distributedCount.c

double:
	$(CC) $(CCFLAGS) -o vectorDouble vectorDouble.c
	mpiexec -n 4 -oversubscribe ./vectorDouble

cyclic:
	$(CC) $(CCFLAGS) -o cyclicSendAndReceive cyclicSendAndReceive.c
	echo "------- running under buffer size -------"
	mpiexec -n 4 -oversubscribe ./cyclicSendAndReceive 1004
	echo "------- running over buffer size -------"
	mpiexec -n 4 -oversubscribe ./cyclicSendAndReceive 1005

dist:
	$(CC) $(CCFLAGS) -o distributedCount distributedCount.c
	mpiexec -n 4 -oversubscribe ./distributedCount

heat:
	$(CC) $(CCFLAGS) -o heatEqn heatEqn.c
	mpiexec -n 4 -oversubscribe ./heatEqn

binary:
	$(CC) $(CCFLAGS) -o reverseBinaryTree reverseBinaryTree.c
	mpiexec -n 4 -oversubscribe ./reverseBinaryTree

clean:
	rm vectorAddition vectorDouble cyclicSendAndReceive distributedCount heatEqn reverseBinaryTree


