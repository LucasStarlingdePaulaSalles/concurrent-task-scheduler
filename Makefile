IDIR=lib
CC=g++
CFLAGS=-g -Wall -lpthread
TFLAGS=-g -Wall -I$(IDIR)

ODIR=obj
SDIR=src

TARGET=main

# _DEPS = SLR-parser.hpp grammar-constants.hpp
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


# Test object
TESTER=test

_DEPS_T = oven-queue.hpp
DEPS_T = $(patsubst %,$(IDIR)/%,$(_DEPS_T))

_OBJ_T = oven-queue.o test.o
OBJ_T = $(patsubst %,$(ODIR)/%,$(_OBJ_T))


$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(TFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(TFLAGS)


test:
$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS_T)
	$(CC) -c -o $@ $< $(TFLAGS)
$(TESTER): $(OBJ_T)
	$(CC) -o $@ $^ $(TFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(TARGET)
	rm -f $(TESTER) 2> /dev/null
