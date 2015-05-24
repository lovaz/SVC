#ifndef CLIENTAPP_HPP
#define CLIENTAPP_HPP

#include "AudioInController.hpp"
#include "AudioOutController.hpp"
#include "NetworkController.hpp"
#include <thread>

class ClientApp
{
private:
	
	AudioOutController* audioPlayer;
	AudioInController* audioRecorder;
	NetworkController* networkController;
	BlockingQueue<Sample> sendQueue;
	BlockingQueue<Sample> receiveQueue;
	bool serverConnected;
	bool hostConnected;
	bool callInProgres;
	int serverPort;
	char* addr;
	void connectToHost(std::string addr, int port);
	void connectToServer();

public:
	ClientApp(int port);
	~ClientApp();
	void commandHandler();
	void initializeEnvironment();
	void startCallViaNet();
	void endCallViaNet();
	void startCallViaServer();
	void endCallViaServer();
	void registerUser();
	void showCommands();
};





#endif
