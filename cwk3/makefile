#
# This makefile should run on the School machines or on a Mac.
# For School machines, you will also need to load required modules;
# see the instructions towards the end of Lecture 14 for details,
# or post a query to the Teams page for this module.
#
EXE = cwk3
OS = $(shell uname)

ifeq ($(OS), Linux)
	CC = nvcc
	LIBS = -lOpenCL
	CCFLAGS = 
endif

ifeq ($(OS), Darwin)
	CC = gcc
	CCFLAGS = -Wall
	LIBS = -framework OpenCL
endif

all:
	$(CC) $(LIBS) $(CCFLAGS) -o $(EXE) cwk3.c
