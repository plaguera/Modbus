CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall

OBJECTS = Main.o ModbusServer.o ModbusTCP.o Util.o

main: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJECTS): ModbusServer.hpp ModbusTCP.hpp Util.hpp

clean:
	rm *.o main