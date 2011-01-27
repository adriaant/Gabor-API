include makeinclude

#########################

SRCS = `ls *.cpp` `ls ../../src/*.cpp`

APIobj = obj/*.o

LIBRARIES = ../lib/libgabor.a

recur: 	makeinclude
	@for dir in $(SRCS); do\
		echo "    $$dir";\
		$(CC) $(OPTIONS) $(INCLUDE_DIR) -c $$dir;\
		mv *.o $(LO);\
	done
	@echo
	make $(LIBRARIES)
	@echo done

all:	recur

../lib/libgabor.a: $(APIobj) 
	@echo -- making library --
	$(AR) -r $@ $(APIobj)
	@echo -- running ranlib --
	ranlib $(LIBRARIES)

clean:
	@echo -- removing libraries and objects --
	$(RM) $(LIBRARIES)
	$(RM) $(LO)*.o || break;
	@echo done

clock:
	$(TOUCH) Makefile
	$(TOUCH) makeinclude
	@echo done
