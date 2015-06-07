// SVC - Simple Voice Communicator 
// Maszyna stanowa aplikacji
// autor: Marcin Frankowski


#include "ClientApp.hpp"



typedef void* (ClientApp::*ClientAppPtr)(void);
typedef void* (*PthreadPtr)(void*);

ClientApp::ClientApp(int port)
{
	audioController = new AudioController();
	networkController = new NetworkController(this);
	serverPort = port;
	serverConnected = false;
	natConnected = false;
	hostConnected = false;
	callInProgres = false;
}

ClientApp::~ClientApp()
{
    delete audioController;
    delete networkController;
}


void ClientApp::connectToHostEvent()
{
	std::string connPort;
	std::string connAddr;
	std::string login;
	if(serverConnected == true)
	{
		std::cout << "Podaj login odbiorcy:" << std::endl;
		std::getline(std::cin, login);
		connectToHostViaServer(login);
	}
	if(serverConnected ==  false) 
	{
		std::cout << "Podaj adres IP:" << std::endl;
		std::getline(std::cin, connAddr);
		std::cout << "Podaj port:" << std::endl;
		std::getline(std::cin, connPort);
		if(checkPort(connPort) != 0)
		{
			std::cout<<"Niepoprawny port!"<<std::endl;
			return;
		}
		connectToHost(connAddr, std::stoi(connPort));
	}
}

void ClientApp::connectToServerEvent()
{
	std::string connPort;
	std::string connAddr;
	std::string login;
    if(hostConnected == true)
    {
      	return;
    }
    if(serverConnected == false)
    {
       	std::cout << "Podaj adres IP:" << std::endl;
		std::getline(std::cin, connAddr);
		std::cout << "Podaj port:" << std::endl;
		std::getline(std::cin, connPort);
		if(checkPort(connPort) != 0)
		{
			std::cout<<"Niepoprawny port!"<<std::endl;
			return;
		}
		std::cout << "Podaj login:" << std::endl;
		std::getline(std::cin, login);
		if(connectToServer(connAddr, std::stoi(connPort), login) == 0)
		{
			serverConnected = true;
		}
		else
		{
			serverConnected = false;
		}
    }
}

void ClientApp::logoutEvent()
{
	networkController->logoutFromServer();
	serverConnected = false;
	networkController->shutdownTcpConnection();
}

void ClientApp::startCallEvent()
{
	if(callInProgres == false && hostConnected == true)
	{
		if(networkController->startUDPNetTransmission() != 0)
		{
			return;
		}
		else
		{
			startCallViaNet();
			callInProgres = true;
		}
	}
	if(callInProgres == false && natConnected == true)
	{
		if(networkController->startUDPNatTransmission() != 0)
		{
			return;
		}
		else
		{
			startCallViaNat();
			callInProgres = true;
		}
	}
}

void ClientApp::endCallEvent()
{
	if(hostConnected == true)
	{
		endCallViaNet();
		std::cout<<"Zakończyłeś rozmowę"<<std::endl;
		return;
	}
	if(natConnected == true)
	{
		endCallViaNat();
		std::cout<<"Zakończyłeś rozmowę"<<std::endl;
		return;
	}
}

void ClientApp::helpEvent()
{
	std::cout<<"******************************************"<<std::endl;
	std::cout<<"************AVAILABLE COMMANDS************"<<std::endl;
	std::cout<<"******************************************"<<std::endl;
	std::cout<<"* connect             starts connection  *"<<std::endl;
	std::cout<<"* start call          starts call        *"<<std::endl;
	std::cout<<"* end call            ends call          *"<<std::endl;
	std::cout<<"* connect to server   connects to server *"<<std::endl;
	std::cout<<"* logout              leaves server      *"<<std::endl;
	std::cout<<"* help                shows this list    *"<<std::endl;
	std::cout<<"* exit                turns off app      *"<<std::endl;
	std::cout<<"* list                shows logged users *"<<std::endl;
	std::cout<<"******************************************"<<std::endl;
}

void ClientApp::exitEvent()
{
	if(serverConnected == true)
	{
		networkController->logoutFromServer();
	}
    if(callInProgres == true)
	{
		if(hostConnected == true)
			networkController->endNetConnection();
		if(natConnected == true)
			networkController->endNatConnection();
		networkController->closeUDPSockets();
		audioController->closeStream();
		audioController->terminateAudio();
	}
	else
	{
		networkController->shutdownTcpConnection();
		networkController->closeUDPSockets();
		audioController->stopRecordThread();
		audioController->stopPlayThread();
		audioController->closeStream();
		audioController->terminateAudio();
	}
	exit(0);
}


void ClientApp::listEvent()
{
	if(serverConnected == true)
	{
		networkController->getUsersList();
	}
	else
	{
		std::cout<<"Nie jesteś podłączony do serwera"<<std::endl;
	}
}

bool ClientApp::isBusy()
{
	if(serverConnected || natConnected || hostConnected || callInProgres)
	{
		return true;
	}
	return false;
}

void ClientApp::commandThread(void)
{
	ClientAppPtr t = &ClientApp::commandHandler;
	PthreadPtr p = *(PthreadPtr*)&t;

	pthread_create(&thread, NULL, p, this);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

void* ClientApp::commandHandler(void)
{
	std::string input;
	std::map<std::string, int> mapStringValues;
	mapStringValues["connect to server"] = 1;
	mapStringValues["connect"] = 2;
	mapStringValues["logout"] = 3;
	mapStringValues["start call"] = 4;
	mapStringValues["end call"] = 5;
	mapStringValues["help"] = 6;
	mapStringValues["exit"] = 7;
	mapStringValues["list"] = 8;
	std::cin.clear();

	while(1)
	{
		std::getline(std::cin, input);

        switch(mapStringValues[input])
        {
        	case 1:  //connect to server
        	connectToServerEvent();
        	break;

        	case 2:  //connect to host
        	connectToHostEvent();
        	break;

        	case 3:  //disconnect from server
        	logoutEvent();
        	break;

        	case 4:  //call
        	startCallEvent();
			break;

        	case 5: //end call
        	endCallEvent();
        	break;

        	case 6: //help
        	helpEvent();
        	break;

        	case 7:	//exit
        	exitEvent();
			break;

			case 8: //list users on server
			listEvent();
			break;
        }
	}
	return (void*)0;
}

void ClientApp::initializeEnvironment()
{
	audioController->initializeAudio();
	audioController->openStream();
	if(networkController->createUdpSocket(50002) != 0)
	{
		exit(-200);
	}
	if(networkController->initTcpServer(serverPort) != 0)
	{
		exit(-100);
	}
	networkController->acceptTcpConnections(serverPort);
}


int ClientApp::connectToServer(std::string addr, int port, std::string logg)
{
	char* caddr = &addr[0u];
	char* login = &logg[0u];
	int ret = networkController -> connectServer(port, caddr, login);
	if(ret == 0)
	{
		std::cout<<"Jesteś dostępny na serwerze"<<std::endl;
		return ret;
	}
	else if(ret < 0)
	{
		std::cout<<"Nie udało się połączyć z serwerem"<<std::endl;
		return ret;
	}
	return -1;
}

int ClientApp::connectToHost(std::string addr, int port)
{
	char* caddr = &addr[0u];
	int ret = networkController->connectHost(port, caddr, "ipv4");
	if(ret == 0)
	{
		std::cout<<"Gotowy aby zacząć rozmowę"<<std::endl;
		hostConnected = true;
		return ret;
	}
	else if(ret == 1)
	{
		std::cout<<"Połączenie odrzucono"<<std::endl;
		hostConnected = false;
		return ret;
	}
	else if(ret == -2)
	{
		std::cout<<"Niepoprawny adres!\n";	
		hostConnected = false;
		return ret;
	}
	else if(ret == -1)
	{
		std::cout<<"Nie można połączyć\n";	
		hostConnected = false;
		return ret;
	}
	else if(ret == 2)
	{
		std::cout<<"Zajęte!\n";	
		hostConnected = false;
		return ret;
	}
	return -1;
}


int ClientApp::connectToHostViaServer(std::string login)
{
	int ret = networkController->callPeerRequest(login);
	if(ret == 0)
	{
		std::cout<<"Gotowy aby rozpocząć rozmowę"<<std::endl;
		natConnected = true;
	}
	else if (ret == 1)
	{
		std::cout<<"Połączenie odrzucone"<<std::endl;
		natConnected = false;
	}
	else if (ret == 2)
	{
		std::cout<<"Użytkownik "<<login<<" nie jest teraz dostępny"<<std::endl;
		natConnected = false;
	}
	else
	{
		exit(401);
	}
	return 0;
}


void ClientApp::startCallViaNet()
{
	audioController->startStream();
	audioController->startPlayThread();
	audioController->startRecordThread();
	networkController->startSendUDPThread();
	networkController->startRecvUDPThread();
	std::thread udpRecvThread(&NetworkController::udpRecv, networkController, std::ref(receiveQueue),0);
    std::thread playAudioThread(&AudioController::playAudio, audioController,std::ref(receiveQueue));
    std::thread udpSendThread(&NetworkController::udpSend, networkController, std::ref(sendQueue),0);
    std::thread recordAudioThread(&AudioController::recordAudio, audioController, std::ref(sendQueue));
    udpRecvThread.detach();
    playAudioThread.detach();
    udpSendThread.detach();
    recordAudioThread.detach();
    callInProgres = true;
}

void ClientApp::endCallViaNet()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	networkController->endNetConnection();
	callInProgres = false;
	hostConnected = false;
	serverConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
}

void ClientApp::stopNetTransmission()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	callInProgres = false;
	hostConnected = false;
	serverConnected = false;
	natConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
	std::cout<<"Twój rozmówca rozłączył się"<<std::endl;
}


void ClientApp::startCallViaNat()
{
	audioController->startStream();
	audioController->startPlayThread();
	audioController->startRecordThread();
	networkController->startSendUDPThread();
	networkController->startRecvUDPThread();
	std::thread udpRecvThread(&NetworkController::udpRecv, networkController, std::ref(receiveQueue),1);
    std::thread playAudioThread(&AudioController::playAudio, audioController,std::ref(receiveQueue));
    std::thread udpSendThread(&NetworkController::udpSend, networkController, std::ref(sendQueue),1);
    std::thread recordAudioThread(&AudioController::recordAudio, audioController, std::ref(sendQueue));
    udpRecvThread.detach();
    playAudioThread.detach();
    udpSendThread.detach();
    callInProgres = true;
    recordAudioThread.detach();
}

void ClientApp::endCallViaNat()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	networkController->endNatConnection();
	callInProgres = false;
	hostConnected = false;
	natConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
}

void ClientApp::stopNatTransmission()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	callInProgres = false;
	hostConnected = false;
	natConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
	std::cout<<"Twój rozmówca rozłączył się"<<std::endl;
}



int ClientApp::acceptCall(std::string ip)
{
	threadCancel();
	std::string trash;
	std::string in;
	std::cout << "Czy rozpocząć rozmowę z "<<ip<<"? (t/n)" << std::endl;
	std::getline(std::cin, trash);
	std::cin.clear();
	std::getline(std::cin, in);
	if(in == "t")
	{
		std::cout<<"Zaakceptowano, gotowy aby rozpocząć"<<std::endl;
		hostConnected = true;
		return 0;
	}
	else
	{
		std::cout<<"Odrzucono"<<std::endl;
		hostConnected = false;
		return -1;
	}
}

int ClientApp::acceptCall(char* login)
{
	threadCancel();
	std::string trash;
	std::string in;
	std::cout << "Czy rozpocząć rozmowę z "<<login<<"? (t/n)" << std::endl;
	std::getline(std::cin, trash);
	std::cin.clear();
	std::getline(std::cin, in);
	if(in == "t")
	{
		std::cout<<"Zaakceptowano, gotowy aby rozpocząć"<<std::endl;
		natConnected = true;
		return 0;
	}
	else
	{
		std::cout<<"Odrzucono"<<std::endl;
		natConnected = false;
		return -1;
	}
}

void ClientApp::threadCancel()
{
	void* res;
	pthread_cancel(thread);
	pthread_join(thread, &res);
}



int ClientApp::checkPort(std::string port)
{
	std::istringstream in(port);
	unsigned short i;
	if(in >> i && in.eof())
	{
		return 0;
	}
	else
	{
		return -1;
	}		
}
	