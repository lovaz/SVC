#include "ClientApp.hpp"
#include <map>


ClientApp::ClientApp(int port)
{
	audioPlayer = new AudioController();
	audioRecorder = new AudioController();
	networkController = new NetworkController(this);
	serverPort = port;
	//initializeEnvironment();
}

ClientApp::~ClientApp()
{
    
}

void ClientApp::commandHandler()
{
	//initializeEnvironment();
	std::string input;
	int connPort;
	std::string connAddr;
	std::map<std::string, int> mapStringValues;
	mapStringValues["connect to server"] = 1;
	mapStringValues["connect to host"] = 2;
	mapStringValues["disconnect from server"] = 3;
	mapStringValues["start call"] = 4;
	mapStringValues["end call"] = 5;
	mapStringValues["set TCP port"] = 6;


	while(true)
	{
		std::cout << "Wpisz polecenie" << std::endl;
		std::getline(std::cin, input);


        switch(mapStringValues[input])
        {
        	case 1:  //connect to server
        		//connectToServer();
        		std::cout << "TODO: connect to server" << std::endl;
        		break;

        	case 2:  //connect to host
        	
        		if(serverConnected == true || hostConnected == true)
        		{
        			std::cout<<"Jesteś już połączony"<<std::endl;
        			break;
        		}
        		std::cout << "Podaj adres IP:" << std::endl;
        		std::cin >> connAddr;
        		std::cout << "Podaj port:" << std::endl;
        		std::cin >> connPort;
        		connectToHost(connAddr, connPort);
        		break;

        	case 3:  //disconnect from server
        		std::cout << "TODO: disconnect from server" << std::endl;
        		break;

        	case 4:  //call
        		networkController->udpConnect();
        		startCallViaNet();
				break;

        	case 5:  //end call
        		endCallViaNet();
        		break;
			case 6:  //end call
				std::cout << "Podaj port nasłuchiwania:" << std::endl;
				std::cin >> serverPort;
        		break;
        }

	}
}

void ClientApp::initializeEnvironment()
{
	audioPlayer->initializeAudio();
	audioRecorder->initializeAudio();
	callInProgres = false;
	networkController->tcpServer(serverPort);
	std::thread tcpRecvThread(&NetworkController::tcpRecv, networkController);
	tcpRecvThread.detach();
}

void ClientApp::connectToHost(std::string addr, int port)
{
	char* caddr = &addr[0u];
	if(networkController->tcpClient(port, caddr, "ipv4") == 0)
	{
		std::cout<<"Gotowy aby zacząć rozmowę"<<std::endl;
	}
	else
	{
		std::cout<<"Nie udało się połączyć"<<std::endl;
	}
}


void ClientApp::startCallViaNet()
{
	audioPlayer->openStream();
	audioRecorder->openStream();
	audioPlayer->startThread();
	audioRecorder->startThread();
	networkController->startSendUDPThread();
	networkController->startRecvUDPThread();
    std::thread udpRecvThread(&NetworkController::udpRecv, networkController, std::ref(receiveQueue));
    std::thread playAudioThread(&AudioController::playAudio, audioPlayer,std::ref(receiveQueue));
    std::thread udpSendThread(&NetworkController::udpSend, networkController, std::ref(sendQueue));
    std::thread recordAudioThread(&AudioController::recordAudio, audioRecorder, std::ref(sendQueue));
    udpRecvThread.detach();
    playAudioThread.detach();
    udpSendThread.detach();
    recordAudioThread.detach();
}

void ClientApp::endCallViaNet()
{
	audioPlayer->stopThread();
	audioRecorder->stopThread();
	networkController->stopSendUDPThread();
	networkController->stopRecvUDPThread();
	if(networkController->shutdownUdpConnection() != 0)
	{
		std::cout<<"Wystąpił problem z zamknięciem gniazd UDP, uruchom ponownie"<<std::endl;
	}
	if(networkController->shutdownTcpConnection() != 0)
	{
		std::cout<<"Wystąpił problem z zamknięciem gniazda TCP, uruchom ponownie"<<std::endl;
	}
	std::cout<<"Poprawnie rozłączono"<<std::endl;
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

void ClientApp::registerUser()
{
		//TODO LATER	
}

