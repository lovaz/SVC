#ifndef CLIENTAPP_HPP
#define CLIENTAPP_HPP

#include "AudioController.hpp"
#include "NetworkController.hpp"
#include <thread>

class ClientApp
{
private:
	
	AudioController* audioPlayer;
	AudioController* audioRecorder;
	NetworkController* networkController;
	BlockingQueue<Sample> sendQueue;
	BlockingQueue<Sample> receiveQueue;
	bool serverConnected;
	bool hostConnected;
	bool callInProgres;
	int serverPort;
	char* addr;
	void startCallViaNet();
	void endCallViaNet();
	void connectToHost(std::string addr, int port);
	void connectToServer();
	void startCallViaServer();
	void endCallViaServer();
	void registerUser();

	
public:
	ClientApp(int port);
	~ClientApp();
	void commandHandler();
		void initializeEnvironment();
};





#endif
