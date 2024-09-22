#include <iostream>
#include <NetLib/Message.h>
#include <NetLib/NetClient.h>

enum class MessageType : uint32_t
{
	FireBullet,
	MovePlayer
};

class Customclient : public Client<MessageType>
{
public:
	bool FireBullet(float x, float y)
	{
		Message<MessageType> msg;
		msg.Header.ID = MessageType::FireBullet;
		msg << x << y;
		Send(msg);
	}
};

void main()
{
	Message<MessageType> msg;
	msg.Header.ID = MessageType::FireBullet;

	int a = 1;
	bool b = true;
	float c = 9.4;

	struct
	{
		float x;
		float y;

	} d[5];

	msg << a << b << c << d;

	a = 13;
	b = 0;
	c = 923.4;

	msg >> d >> c >> b >> a;

	std::cout << a << b << c << d;

}