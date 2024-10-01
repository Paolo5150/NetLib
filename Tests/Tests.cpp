#include "NetLib/UDPPacketAssembler.h"
#include "TestUDPPacket.h"
#include "TestUDPPacketAssembler.h"
#include "TestUDPMessager.h"

void main()
{
	std::cout << "\n\n|----------------|\n";
	std::cout << " Running some incredible tests...";
	std::cout << "\n|----------------|\n\n";
	TestUDPPacket();
	TestUDPPacketAssembler();
	TestUDPPacketAssembler2();
	TestUDPPacketAssembler3();
	TestUDPMessager();

	std::cout << "\n\n|----------------|\n";
	std::cout << " All tests passed";
	std::cout << "\n|----------------|\n\n";
}