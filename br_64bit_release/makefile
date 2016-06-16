CC	= g++-3.4

CFLAGS	= -O3 -Wno-deprecated -DNDEBUG  -pthread -I./stlport

SRCS	= BBox.cpp Boundary.cpp BoxRouter.cpp Design.cpp flute.cpp Grid.cpp GRouter.cpp ILPSolver.cpp Layer.cpp Net.cpp Object.cpp Param.cpp Pin.cpp Point.cpp Segment.cpp Util.cpp Wire.cpp MazeHeap.cpp

OBJS = $(SRCS:.cpp=.o)

EXE	= br_ispd07.x

.SUFFIXES: .cc .cpp .o .x

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L. -lglpk -lstlport -lcurses

.cpp.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CC) $(OFLAGS) -c $<

clean:
	rm -f $(OBJS) *~ $(EXE) core

copy:
	cp ../br_ispd07_64bit/*.cpp .
	cp ../br_ispd07_64bit/*.h .

dmp2res:
	$(CC) -O3 -o dmp2res.x dmp2res.cpp

res2dmp:
	$(CC) -O3 -o res2dmp.x res2dmp.cpp

