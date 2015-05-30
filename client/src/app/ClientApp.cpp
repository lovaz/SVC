#include "ClientApp.hpp"
#include <map>


typedef void* (ClientApp::*ClientAppPtr)(void);
typedef void* (*PthreadPtr)(void*);

ClientApp::ClientApp(int port)
{
	audioController = new AudioController();
	networkController = new NetworkController(this);
	serverPort = port;
	serverConnected = false;
	hostConnected = false;
	callInProgres = false;
}

ClientApp::~ClientApp()
{
    delete audioController;
    delete networkController;
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
	mapStringValues["connect to host"] = 2;
	mapStringValues["disconnect from server"] = 3;
	mapStringValues["start call"] = 4;
	mapStringValues["end call"] = 5;
	mapStringValues["help"] = 7;
	mapStringValues["exit"] = 8;
	std::cin.clear();

	while(1)
	{
		std::getline(std::cin, input);

        switch(mapStringValues[input])
        {
        	case 1:  //connect to server
        	conToServEvent();
        	break;

        	case 2:  //connect to host
        	conToHostEvent();
        	break;

        	case 3:  //disconnect from server
        	disconFromServerEvent();
        	break;

        	case 4:  //call
        	startCallEvent();
			break;

        	case 5: //end call
        	endCallEvent();
        	break;

        	case 7: //help
        	helpEvent();
        	break;

        	case 8:	//exit
        	exitEvent();
			break;
        }
	}
	return (void*)0;
}

void ClientApp::initializeEnvironment()
{
	audioController->initializeAudio();
	audioController->openStream();
	if(networkController->createUdpSockets(50002) != 0)
	{
		exit(-200);
	}
	if(networkController->initTcpServer(serverPort) != 0)
	{
		exit(-100);
	}
	networkController->acceptTcpConnections(serverPort);
}

int ClientApp::connectToHost(std::string addr, int port)
{
	char* caddr = &addr[0u];
	int ret = networkController->tcpConnect(port, caddr, "ipv4");
	if(ret == 0)
	{
		std::cout<<"Gotowy aby zacząć rozmowę"<<std::endl;
		return ret;
	}
	else if(ret == 1)
	{
		std::cout<<"Połączenie odrzucono"<<std::endl;
		return ret;
	}
	else if(ret < 0)
	{
		std::cout<<"Nie udało się połączyć"<<std::endl;
		return ret;
	}
	return -1;
}


void ClientApp::startCallViaNet()
{
	callInProgres = true;
	audioController->startStream();
	audioController->startPlayThread();
	audioController->startRecordThread();
	networkController->startSendUDPThread();
	networkController->startRecvUDPThread();
	std::thread udpRecvThread(&NetworkController::udpRecv, networkController, std::ref(receiveQueue));
    std::thread playAudioThread(&AudioController::playAudio, audioController,std::ref(receiveQueue));
    std::thread udpSendThread(&NetworkController::udpSend, networkController, std::ref(sendQueue));
    std::thread recordAudioThread(&AudioController::recordAudio, audioController, std::ref(sendQueue));
    udpRecvThread.detach();
    playAudioThread.detach();
    udpSendThread.detach();
    recordAudioThread.detach();
}

void ClientApp::endCallViaNet()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->endConnection();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	networkController->stopRecvTCPThread();
	callInProgres = false;
	hostConnected = false;
	serverConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
}

void ClientApp::stopAudio()
{
	audioController->stopPlayThread();
	audioController->stopRecordThread();
	audioController->stopStream();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	networkController->stopRecvTCPThread();
	callInProgres = false;
	hostConnected = false;
	serverConnected = false;
	sendQueue.clear();
	receiveQueue.clear();
	std::cout<<"Twój rozmówca rozłączył się"<<std::endl;
}


void ClientApp::startCallViaServer()
{

}

void ClientApp::endCallViaServer()
{

}

void ClientApp::connectToServer()
{
		//TODO LATER
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
		hostConnected = true;
		return 0;
	}
	else
	{
		hostConnected = false;
		return -1;
	}
}

void ClientApp::threadCancel()
{
	void* res;
	pthread_cancel(thread);
	pthread_join(thread, &res);
}

void ClientApp::conToHostEvent()
{
	std::string connPort;
	std::string connAddr;
	if(serverConnected == true)
	{
		return;
	}
	if(hostConnected == false)
	{
		std::cout << "Podaj adres IP:" << std::endl;
		std::getline(std::cin, connAddr);
		std::cout << "Podaj port:" << std::endl;
		std::getline(std::cin, connPort);
		bool containsNumbers = connPort.find_first_not_of("0123456789");
		if(containsNumbers != std::string::npos)
		{
			std::cout<<"Niepoprawny adres!\n";
			return;
		}

		int ret = connectToHost(connAddr, std::stoi(connPort));
		if(ret == 0)
		{
			hostConnected = true;
		}
		if(ret == -2)
		{
			std::cout<<"Niepoprawny adres!\n";	
			hostConnected = false;
		}
		if(ret == -1)
		{
			std::cout<<"Nie można połączyć\n";
			hostConnected = false;
		}
	}
}

void ClientApp::conToServEvent()
{
    if(hostConnected == true)
    {
      	return;
    }
    if(serverConnected == false)
    {
       	std::cout << "TODO: connect to server" << std::endl;
    }
}

void ClientApp::disconFromServerEvent()
{
	std::cout << "TODO: disconnect from server" << std::endl;
}

void ClientApp::endCallEvent()
{
	if(callInProgres == true || hostConnected == true || serverConnected == true)
	{
		endCallViaNet();
	}
}

void ClientApp::startCallEvent()
{
	if(callInProgres == false && hostConnected == true)
	{
		if(networkController->udpConnect() != 0)
		{
			return;
		}
		else
		{
			startCallViaNet();
			callInProgres = true;
		}
	}
}

void ClientApp::helpEvent()
{
	std::cout<<"******************************"<<std::endl;
	std::cout<<"******Available commands******"<<std::endl;
	std::cout<<"******************************"<<std::endl;
	std::cout<<"* connect to host            *"<<std::endl;
	std::cout<<"* start call                 *"<<std::endl;
	std::cout<<"* end call                   *"<<std::endl;
	std::cout<<"* connect to server          *"<<std::endl;
	std::cout<<"* disconnect from server     *"<<std::endl;
	std::cout<<"* exit                       *"<<std::endl;
	std::cout<<"******************************"<<std::endl;
}

void ClientApp::exitEvent()
{
    if(callInProgres == true)
	{
		networkController->endConnection();
		networkController->shutdownUdpConnection();
		audioController->closeStream();
		audioController->terminateAudio();
	}
	else
	{
		networkController->shutdownTcpConnection();
		networkController->shutdownUdpConnection();
		audioController->stopRecordThread();
		audioController->stopPlayThread();
		audioController->closeStream();
		audioController->terminateAudio();
		networkController->shutdownUdpConnection();
	}
	exit(0);
}
	

void ClientApp::checkCIN(std::string msg)
{
	std::cout << msg << ": "
      << "\ncin.rdstate(): " << std::cin.rdstate()
      << "\n    cin.eof(): " << std::cin.eof()
      << "\n   cin.fail(): " << std::cin.fail()
      << "\n    cin.bad(): " << std::cin.bad()
      << "\n   cin.good(): " << std::cin.good()<<std::endl;
}
