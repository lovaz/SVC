#include "ClientApp.hpp"
#include <map>


ClientApp::ClientApp(int port)
{
	audioPlayer = new AudioOutController();
	audioRecorder = new AudioInController();
	networkController = new NetworkController(this);
	serverPort = port;
}

ClientApp::~ClientApp()
{
    
}

void ClientApp::commandHandler()
{
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
	mapStringValues["help"] = 7;
	mapStringValues["exit"] = 8;
	std::cout << "Wpisz polecenie. Aby wyświetlić dostępne komendy wpisz 'help'" << std::endl;

	while(true)
	{
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
        	case 7:
        		showCommands();
        		break;
        	case 8:
        		audioPlayer->stopThread();
				audioRecorder->stopThread();
				networkController->stopSendUDPThread();
				networkController->stopRecvUDPThread();
				networkController->shutdownUdpConnection();
				networkController->shutdownTcpConnection();
				exit(0);
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
    std::thread playAudioThread(&AudioOutController::playAudio, audioPlayer,std::ref(receiveQueue));
    std::thread udpSendThread(&NetworkController::udpSend, networkController, std::ref(sendQueue));
    std::thread recordAudioThread(&AudioInController::recordAudio, audioRecorder, std::ref(sendQueue));
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
	networkController->shutdownUdpConnection();
	networkController->shutdownTcpConnection();
	networkController->tcpServer(serverPort);
	std::cout<<"Koniec rozmowy"<<std::endl;
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

void ClientApp::showCommands()
{
	std::cout<<"******************************"<<std::endl;
	std::cout<<"******Available commands******"<<std::endl;
	std::cout<<"******************************"<<std::endl;
	std::cout<<"* connect to host            *"<<std::endl;
	std::cout<<"* start call                 *"<<std::endl;
	std::cout<<"* end call                   *"<<std::endl;
	std::cout<<"* set TCP port               *"<<std::endl;
	std::cout<<"* exit                       *"<<std::endl;
	std::cout<<"******************************"<<std::endl;
}
