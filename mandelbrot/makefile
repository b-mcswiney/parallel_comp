#
# A simple makefile that compiles GLFW on Linux or Macs.
#
EXE = Mandelbrot_MPI
CC = mpicc
CCFLAGS = -Wall -DGL_SILENCE_DEPRECATION

OS = $(shell uname)

ifeq ($(OS), Linux)
	MSG = If running on a school machine and you get an error \'mpi.h: no such file or directory\', load the MPI mopdule first, e.g. \'module load openmpi\' \(Red Hat\).
	MSG += or \'module load mpi/openmpi-x86_64\' \(Centos\). If you then get an error \'GLFW/glfw3.h: No such file or directory\', try \'module load glfw\'.
	MSG += If running on a school Centos machine \(e.g. feng-linux\), you will also need to change \'-lglfw3\' to \'-lglfw\' in the makefile,
	MSG += otherwise you will get a \'cannot find -glfw3\' error message.
	CCFLAGS += -lm -lX11 -ldl -lGL -lglfw3
endif

ifeq ($(OS), Darwin)
	MSG = Requires GLFW\; current include/lib dirs work for GLFW installed via homebrew but may need to be altered for other distributions.
	CCFLAGS += -lglfw -framework OpenGL -L /usr/local/lib -I /usr/local/include
endif

all:
	@echo $(MSG)
	@echo
	$(CC) -o $(EXE) Mandelbrot_MPI.c $(CCFLAGS) 
