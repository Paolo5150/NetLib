#include <iostream>
#include <NetLib/NetMessage.h>
#include <NetLib/TCPServer.h>
#include <NetLib/UDPMessager.h>
#include "ServerUDP.h"
#include "ServerTCP.h"


void main()
{
	ServerTCP tcpGuy(60000);
	tcpGuy.Start();

	bool key[10] = { 0,0,0,0,0,0,0,0,0,0 };
	bool oldKey[10] = { 0,0,0,0,0,0,0,0,0,0 };

	bool quittime = false;
	while (!quittime)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('0') & 0x8000;
			key[1] = GetAsyncKeyState('1') & 0x8000;
			key[2] = GetAsyncKeyState('2') & 0x8000;
			key[3] = GetAsyncKeyState('3') & 0x8000;
			key[4] = GetAsyncKeyState('4') & 0x8000;
			key[5] = GetAsyncKeyState('5') & 0x8000;
			key[6] = GetAsyncKeyState('6') & 0x8000;
			key[7] = GetAsyncKeyState('7') & 0x8000;
			key[8] = GetAsyncKeyState('8') & 0x8000;
			key[9] = GetAsyncKeyState('9') & 0x8000;
		}

		if (key[0] && !oldKey[0])
		{
			std::cout << "Pressed 0, quitting\n";
			quittime = true;
			break;
		}

		for (int i = 0; i < 10; i++)
		{
			if (key[i] && !oldKey[i])
			{
				//udpGuy.OnKeyPressed(i);
				tcpGuy.OnKeyPressed(i);
			}
			oldKey[i] = key[i];
		}

		tcpGuy.Tick();

	}


}