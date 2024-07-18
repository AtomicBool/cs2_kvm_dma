CXX=g++
CXXFLAGS=-std=c++2a -I./includes/ -I/usr/local/include/opencv4/ -I./sdk/ -L./libs -l:vmm.so -l:leechcore.so -l:leechcore_device_microvmi.so -L/usr/local/lib -l:libopencv_core.so -l:libopencv_dnn.so -Wno-multichar
LIBS=-lm -Wl,--no-as-needed -ldl -lpthread

OUTDIR=./build
OBJDIR=$(OUTDIR)/obj

$(shell mkdir -p $(OBJDIR))
$(shell cp libs/leechcore.so /usr/lib/)
$(shell cp libs/vmm.so /usr/lib/)
$(shell cp libs/libmicrovmi.so /usr/lib/)
$(shell cp libs/leechcore_device_microvmi.so /usr/lib/)
$(shell cp libs/libmemflow_qemu.x86_64.so $(OUTDIR)/)

%.o: %.cpp
	$(CXX) -c -o $(OBJDIR)/$@ $< $(CXXFLAGS)

Memory.o: ./sdk/Memory.cpp
	$(CXX) -c -o $(OBJDIR)/Memory.o ./sdk/Memory.cpp $(CXXFLAGS)

InputManager.o: ./sdk/InputManager.cpp
	$(CXX) -c -o $(OBJDIR)/InputManager.o ./sdk/InputManager.cpp $(CXXFLAGS)

EasyWSClient.o: ./includes/easywsclient.cpp
	$(CXX) -c -o $(OBJDIR)/EasyWSClient.o ./includes/easywsclient.cpp $(CXXFLAGS)

cs_dma: cs_dma.o game.o Memory.o InputManager.o EasyWSClient.o
	$(CXX) -o $(OUTDIR)/$@ $(OBJDIR)/cs_dma.o $(OBJDIR)/game.o $(OBJDIR)/Memory.o $(OBJDIR)/InputManager.o $(OBJDIR)/EasyWSClient.o $(CXXFLAGS) $(LIBS)
	rm -rf ./build/obj/

.PHONY: all
all: cs_dma

.DEFAULT_GOAL := all

clean:
	rm -rf ./build/cs_dma
	rm -rf /usr/lib/vmm.so
	rm -rf /usr/lib/leechcore.so
	rm -rf /usr/lib/leechcore_device_microvmi.so
	rm -rf /usr/lib/libmicrovmi.so
	rm -rf ./build/libmemflow_qemu.x86_64.so
