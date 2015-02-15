#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# g++ -L /usr/lib -l uhd -o e100test test_routines.cpp
# g++ -g -L /usr/lib -l uhd -o rxtest  receiver_test.cpp uhd_utilities.cpp
# g++ -g -L /usr/lib -l uhd -o serial_port_test serial_port_test.cpp
# g++ -pthread -o thread_test thread_test.cpp

# g : Indicates debug mode
# c : Indicates compilation only

IDIR =.
CC =g++
CXXFLAGS = -std=gnu++11 -I$(IDIR)
LINKFLAGS =

OBJDIR = obj
LIBDIR = /usr/lib

LIBS= -lstdc++ -lpthread -lm

$(OBJDIR)/%.o:%.cpp  buffers.h
	$(CC) -c -o $@ $< $(CXXFLAGS)

############### BUFFERS TEST

_OBJ_BT = buffers_test.o
OBJ_BT = $(patsubst %, $(OBJDIR)/%, $(_OBJ_BT))

buffers_test:$(OBJ_BT) 	
	$(CC) -g -L $(LIBDIR)  -o $@ $^  $(LINKFLAGS) $(LIBS)

############### CLEAN UP

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~
