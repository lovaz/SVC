#ifndef CLIENTAPP_HPP
#define CLIENTAPP_HPP

#include "AudioController.hpp"
#include "NetworkController.hpp"
#include <thread>
#include <pthread.h>
#include <stdio.h>


class ClientApp
{
private:
	
	AudioController* audioController;
	NetworkController* networkController;
	BlockingQueue<Sample> sendQueue;
	BlockingQueue<Sample> receiveQueue;
	bool serverConnected;
	bool hostConnected;
	bool callInProgres;
	int serverPort;
	pthread_t thread;
	int connectToHost(std::string addr, int port);
	void connectToServer();

public:

	ClientApp(int port);
	~ClientApp();
	void* commandHandler();
	void initializeEnvironment();
	void startCallViaNet();
	void endCallViaNet();
	void startCallViaServer();
	void endCallViaServer();
	void commandThread();
	void threadCancel();
	void stopAudio();
	void conToHostEvent();
	void conToServEvent();
	void disconFromServerEvent();
	void endCallEvent();
	void startCallEvent();
	void helpEvent();
	void exitEvent();
	void checkCIN(std::string msg);
	int acceptCall(std::string ip);
};





#endif
