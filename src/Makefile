PROG=draw
PNG = /home/Yu/class/cs520/png/125

INCDIR = -I/home/Yu/class/cs520/imageio/
LIB_DIR = /usr/lib64
LIBS    = -L$(LIB_DIR) -L../ -L$(PNG)/lib/ -lglut -lGLU -lGL -lGLEW -ltiff  -lpng12 -lSDL2

IMGIO = /home/Yu/class/cs520/imageio/imageio.o


#source codes
SRCS = $(PROG).cpp
#substitute .cpp by .o to obtain object filenames
OBJS = $(SRCS:.cpp=.o)

CC=g++

$(PROG): $(OBJS)
        $(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@ $(IMGIO)

$(OBJS): $(SRCS)
        $(CC) -c $*.cpp $(INCDIR)

run:
        export LD_LIBRARY_PATH=$(PNG)/lib; ./$(PROG)

clean:
        -rm -f $(OBJS)  *~
