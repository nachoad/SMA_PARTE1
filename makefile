# makefile file for "audioSimple"
# instructions: you can execute once 
# 	make depend
# so that  gccmakedep  tool could detect dependencies on the different files
# if later any #include is added, execute again 'make depend'
# if any other .c file is added, write it in source 


# -Wall performs more rigurous type checking 
MOREFLAGS = -Wall

SOURCE =  confArgs.c audioConfig.c audioConfig.c conf1.c 

conf1:	conf1.o confArgs.o audioConfig.o circularBuffer.o
	gcc $(MOREFLAGS) -o conf1 conf1.o confArgs.o audioConfig.o circularBuffer.o -lm

conf1.o:	conf1.c
	gcc $(MOREFLAGS) -c conf1.c

confArgs.o:	confArgs.c confArgs.h
	gcc $(MOREFLAGS) -c confArgs.c

audioConfig.o:	audioConfig.c audioConfig.h
	gcc $(MOREFLAGS) -c audioConfig.c

circularBuffer.o:	circularBuffer.c circularBuffer.h
	gcc $(MOREFLAGS) -c circularBuffer.c

# Utilities 
tar: clean
	tar cvf audio.tar *
	gzip audio.tar


clean:	
	rm -f *.a *.o *~ core


depend:
	gccmakedep $(CFLAGS) -s '# MAKEDEPEND boundary' $(SOURCE)


# MAKEDEPEND boundary
