.SILENT:

CC = g++ -O4 -funroll-all-loops -fexpensive-optimizations -Wno-deprecated
RM = rm -f
TOUCH = touch

WHERE = `pwd`/
HOME = /Users/adriaant/

MFLAGS = all

INCLUDE_DIR = -I$(WHERE)include/ -I$(WHERE)../include/
LO = $(WHERE)obj/
LL = $(WHERE)lib/

LIBDIRS = -L$(LL)
LIBS = -lgabor

OBJS = $(LO)*.o

EXEC = gaborglobal

SRCS = Gabor.cpp

$(EXEC): 
	@echo -- compiling:
	@for dir in $(SRCS); do\
		echo "    $$dir";\
		$(CC) $(INCLUDE_DIR) -c $$dir;\
		mv *.o $(LO);\
	done
	@echo
	@echo -- making $(EXEC) --
	$(CC) -g $(OBJS) $(LIBDIRS) $(LIBS) -o $@
	@echo -- installing $(EXEC) --
	mv -f $(EXEC) $(HOME)bin/
	@echo done

clean:
	@echo -- cleaning objects and executable --
	-$(RM) $(OBJS) $(EXEC)
	@echo done

clock:
	$(TOUCH) Makefile
	$(TOUCH) makeinclude
	@echo

