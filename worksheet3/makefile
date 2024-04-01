
CC = nvcc
CCFLAGS = -lOpenCL

vadd:
	$(CC) $(CCFLAGS) -o vectorAddition vectorAddition.c
	./vectorAddition
vsub:
	$(CC) $(CCFLAGS) -o vectorSubtraction vectorSubtraction.c
	./vectorSubtraction

overflow:
	$(CC) $(CCFLAGS) -o registerOverflow registerOverflow.c
	./registerOverflow

