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
	bool natConnected;
	bool callInProgres;
	int serverPort;
	pthread_t thread;
	int connectToHost(std::string addr, int port);
	int connectToServer(std::string addr, int port, std::string logg);
	int connectToHostViaServer(std::string login);

public:

	ClientApp(int port);
	~ClientApp();
	void* commandHandler();
	void initializeEnvironment();
	void startCallViaNet();
	void endCallViaNet();
	void startCallViaNat();
	void endCallViaNat();
	void commandThread();
	void threadCancel();
	void stopTransmission();
	void stopNatTransmission();
	void conToHostEvent();
	void conToServEvent();
	void disconFromServerEvent();
	void endCallEvent();
	void startCallEvent();
	void helpEvent();
	void exitEvent();
	void logoutEvent();
	void checkCIN(std::string msg);
	int acceptCall(std::string ip);
	int acceptCall(char* login);
	 void resetApp(){}
};





#endif
