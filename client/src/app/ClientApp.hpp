// SVC - Simple Voice Communicator 
// Maszyna stanowa aplikacji
// autor: Marcin Frankowski


#ifndef CLIENTAPP_HPP
#define CLIENTAPP_HPP

#include "AudioController.hpp"
#include "NetworkController.hpp"
#include <iostream>
#include <ios>
#include <sstream>
#include <thread>
#include <pthread.h>
#include <stdio.h>
#include <map>


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
	void connectToServerEvent();
	void connectToHostEvent();
	void logoutEvent();
	void startCallEvent();
	void endCallEvent();
	void helpEvent();
	void exitEvent();
	void listEvent();

public:

	ClientApp(int port);
	~ClientApp();
	void* commandHandler();
	void initializeEnvironment();
	void startCallViaNet();
	void endCallViaNet();
	void startCallViaNat();
	void endCallViaNat();
	void stopNetTransmission();
	void stopNatTransmission();
	void commandThread();
	void threadCancel();
	int acceptCall(std::string ip);
	int acceptCall(char* login);
	int checkPort(std::string port);
	bool isBusy();
};





#endif
