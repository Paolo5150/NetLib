#include "NetLib/UDPPacketAssembler.h"
#include "TestUDPPacket.h"
#include "TestUDPPacketAssembler.h"
#include "TestUDPReceiver.h"



void main()
{
	TestUDPPacket();
	TestUDPPacketAssembler();
	TestUDPPacketAssembler2();
	TestUDPPacketAssembler3();
	TestUDPReceiver();
	std::cout << "All tests passed";
}