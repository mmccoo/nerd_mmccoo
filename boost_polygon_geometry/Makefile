

EXEC_LIST = show_shape_gtl show_shape_geom test1 test2 test3 test4 test5 test6
all: $(EXEC_LIST)

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
%.o:%.cpp
	g++ -Wall -g -O2 -I. $(DEPFLAGS) -c $< -o $@


show_shape_gtl.o: show_shape.cpp
	g++ -g -c -I.  $(DEPFLAGS) `wx-config --cflags` -o $@ $^

show_shape_geom.o: show_shape.cpp
	g++ -DSHOW_GEOM=1 -g -c -I.  $(DEPFLAGS) `wx-config --cflags` -o $@ $^


HELPER_OBJS = geom_poly_serialize.o gtl_poly_serialize.o poly_utils.o gtl_poly_types.o geom_poly_types.o
show_shape_gtl: show_shape_gtl.o $(HELPER_OBJS)
	g++ `wx-config --libs` $^ -lboost_serialization -o $@

show_shape_geom: show_shape_geom.o  $(HELPER_OBJS)
	g++ `wx-config --libs` $^ -lboost_serialization -o $@

test1: test1_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@

test2: test2_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@

test3: test3_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@

test4: test4_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@

test5: test5_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@

test6: test6_main.o $(HELPER_OBJS)
	g++ $^ -lboost_serialization -o $@



clean:
	rm -f *.o *.Td *.out $(EXEC_LIST)

include $(wildcard *.Td)
