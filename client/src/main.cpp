#include "ClientApp.hpp"
#include "AudioController.hpp"
#include <unistd.h>
#include <thread>
#include "ThreadSafeMap.hpp"
#include <iostream>


int main(int argc, char** argv)
{
    ClientApp* clientApp = new ClientApp(atoi(argv[1]));
    std::system("clear");
    std::cout << "Wpisz polecenie. Aby wyświetlić dostępne komendy wpisz 'help'" << std::endl;
    clientApp->commandThread();
    clientApp->initializeEnvironment();
}

	// AudioController* audioController;
	// audioController = new AudioController();
	// audioController->initializeAudio();
	// audioController->openStream();
	// audioController->startStream();
	// std::thread t1(&AudioController::test1, audioController);
	// std::thread t2(&AudioController::test2, audioController);
	// //audioController->test();
	// t1.join();
	// t2.join();
	// audioController->stopStream();
	// std::cout<<"\n\n\n\n\n\n\nKTEEEEEEEEEEEEEEEEEEEEST\n\n\n\n\n\n\n";
	// audioController->startStream();
	// std::thread t3(&AudioController::test1, audioController);
	// std::thread t4(&AudioController::test2, audioController);
	// t3.join();
	// t4.join();
	// audioController->stopStream();
	// audioController->closeStream();
	// audioController->terminateAudio();


// void producer1(ThreadSafeMap<std::string>& mapka)
// {
//     std::string a = "klucz";
//     std::string w = "wartosc";
//     std::string c = "chuj";
//     std::string n = "nic";
//     std::string b = "xxx";
//     std::string aa = "1klucz";
//     std::string ww = "1wartosc";
//     std::string cc = "1chuj";
//     std::string nn = "1nic";
//     std::string bb = "1xxx";
//     std::cout<<mapka.insert(a, w)<<std::endl;
//     std::cout<<mapka.insert(w, w)<<std::endl;
//     std::cout<<mapka.insert(c, w)<<std::endl;
//     std::cout<<mapka.insert(b, w)<<std::endl;
//     std::cout<<mapka.insert(n, w)<<std::endl;
//     std::cout<<mapka.insert(aa, w)<<std::endl;
//     std::cout<<mapka.insert(ww, w)<<std::endl;
//     std::cout<<mapka.insert(cc, w)<<std::endl;
//     std::cout<<mapka.insert(bb, w)<<std::endl;
//     std::cout<<mapka.insert(nn, w)<<std::endl;
// }

// void producer2(ThreadSafeMap<std::string>& mapka)
// {
//     std::string a = "klucz";
//     std::string w = "wartosc";
//     std::string c = "chuj";
//     std::string n = "nic";
//     std::string b = "xxx";
//     std::string aa = "1klucz";
//     std::string ww = "1wartosc";
//     std::string cc = "1chuj";
//     std::string nn = "1nic";
//     std::string bb = "1xxx";
//     std::cout<<mapka.insert(nn, n)<<std::endl;
//     std::cout<<mapka.insert(a, n)<<std::endl;
//     std::cout<<mapka.insert(w, n)<<std::endl;
//     std::cout<<mapka.insert(ww, n)<<std::endl;
//     std::cout<<mapka.insert(c, n)<<std::endl;
//     std::cout<<mapka.insert(b, n)<<std::endl;
//     std::cout<<mapka.insert(bb, n)<<std::endl;
//     std::cout<<mapka.insert(n, n)<<std::endl;
//     std::cout<<mapka.insert(aa, n)<<std::endl;
//     std::cout<<mapka.insert(cc, n)<<std::endl;
// }




// int main(int argc, char** argv)
// {
//     std::string a = "klucz";
//     std::string w = "wartosc";
//     std::string c = "chuj";
//     std::string n = "nic";
//     ThreadSafeMap<std::string> mapka;
//     std::cout<<mapka.insert(a, w)<<std::endl;
//     std::cout<<mapka.insert(a, w)<<std::endl;
//     std::cout<<mapka.insert(c, w)<<std::endl;
//     std::cout<<mapka.delete_(c)<<std::endl;
//     std::cout<<mapka.insert(c, w)<<std::endl;
//     std::cout<<mapka.delete_(n)<<std::endl;
//     std::thread t1(producer1, std::ref(mapka));
//     std::thread t2(producer2, std::ref(mapka));
//     t1.join();
//     t2.join();
//     std::cout<<mapka.find(a)<<std::endl;
//     std::cout<<mapka.find(w)<<std::endl;
//     std::cout<<mapka.find(c)<<std::endl;
//     std::cout<<mapka.find(n)<<std::endl;
//     std::cout<<"****************************"<<std::endl;
//     mapka.print();

//    //std::map<std::string, std::string> mapa;
//    //std::map<std::string, std::string>::iterator it;

//    // std::cout<<mapa.insert("elo", "chuj")<<std::endl;
//    // std::cout<<mapa.insert("elo", "a")<<std::endl;
//    // std::cout<<mapa.insert("elo", "b")<<std::endl;
//    // std::cout<<mapa.insert("elo", "c")<<std::endl;
//    // std::cout<<mapa.insert(std::pair<std::string, std::string>("elo", "1"))<<std::endl;
//     //std::cout<<mapa.insert(std::pair<std::string, std::string>("elo", "b"))<<std::endl;
//    // std::cout<<mapa.insert(std::pair<std::string, std::string>("elo", "c"))<<std::endl;
//    // mapa.insert("elo", "a");
//     //mapa.insert("elo", "b");
//    // mapa.insert("elo", "c");

//   // std::cout<<"elo ==> " <<mapa.find("elo")->second <<std::endl;
//    return 0;
// }

