CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall

MTCP_OBJECTS = Main.o ModbusServer.o ModbusTCP.o Util.o
T1_OBJECTS = test/Test1.o ModbusServer.o Util.o
T2_OBJECTS = test/Test2.o ModbusServer.o Util.o
T3_OBJECTS = test/Test3.o ModbusServer.o Util.o
default: ModbusTCP

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

ModbusTCP: $(MTCP_OBJECTS)
	$(CXX) $(MTCP_OBJECTS) -o $@ $(CXXFLAGS)

test1: $(T1_OBJECTS)
	$(CXX) $(T1_OBJECTS) -o $@ $(CXXFLAGS)

test2: $(T2_OBJECTS)
	$(CXX) $(T2_OBJECTS) -o $@ $(CXXFLAGS)

test3: $(T3_OBJECTS)
	$(CXX) $(T3_OBJECTS) -o $@ $(CXXFLAGS)

tests: test1 test2 test3

clean:
	-rm -f $(MTCP_OBJECTS) $(T1_OBJECTS) $(T2_OBJECTS) $(T3_OBJECTS)
	-rm -f ModbusTCP test1 test2 test3