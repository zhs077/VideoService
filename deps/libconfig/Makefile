CC=gcc
CFLAGS+=-I.

OBJS= configlib.o


all: $(OBJS)
	@ar rscv libconfiglib.a $(OBJS)
.c.o:
	$(CC) -c $< $(CFLAGS)

install:all
	@cp libconfiglib.a ${EPGIS_INSTALL_LIB_PATH}
	@cp configlib.h ${EPGIS_INSTALL_INCLUDE_PATH}
test:
	$(CC) -o foo configlib.c -DTEST_LIB
	@./foo

.PHONY : clean
clean :
	@rm -f $(OBJS) libconfiglib.a foo
