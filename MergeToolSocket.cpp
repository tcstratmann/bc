/*   Bridge Command 5.0 - MergeToolSocket Integration
     Copyright (C) 2016 Dierk Brauer
     dierk.brauer@uni-oldenburg.de
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/time.h>
#include "MergeToolSocket.hpp"

#ifdef __WIN32__
# include <winsock2.h>
# include <windows.h>
# include <unistd.h>
# include <WS2tcpip.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

//#include "sio_client.h"

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include <math.h>       /* atan2 */
#include <thread>

const int DEFAULT_PORT = 5006;

using namespace std;

bool connected = false;
long const SHIP1_START_TIME = 360000;  // 6min
long const SHIP2_START_TIME = 1320000;  // 22min
MergeToolSocket::MergeToolSocket() //Constructor
{
    std::cout << "Init MergeToolSocket\n";

    #ifdef __WIN32__
       WORD versionWanted = MAKEWORD(2, 2);
       WSADATA wsaData;
       int wsaerr = WSAStartup(versionWanted, &wsaData);
    if (wsaerr != 0)
    {
        /* Tell the user that we could not find a usable WinSock DLL.*/

        printf("The Winsock dll not found!\n");

        exit(0);
    }
    else
    {
           printf("The Winsock dll found!\n");

           printf("The status: %s.\n", wsaData.szSystemStatus);
    }
     /* Confirm that the WinSockerDLL supports 2.2.        */

    /* Note that if the DLL supports versions greater    */

    /* than 2.2 in addition to 2.2, it will still return */

    /* 2.2 in wVersion since that is the version we      */

    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)

    {

        /* Tell the user that we could not find a usable WinSock DLL.*/

        printf("The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion));

        WSACleanup();

        exit(0);
    }
    else

    {
           printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion));

           printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
    }
    #endif

    model=0; //Not linked at the moment

    startServer(DEFAULT_PORT);
}

MergeToolSocket::~MergeToolSocket() //Destructor
{
    //shut down socket.io connection
	// TODO implement

    std::cout << "Shut down MergeToolSocket connection\n";
    close(newsockfd);
    close(sockfd);

    closesocket(newsockfd);
    closesocket(sockfd);

}

bool MergeToolSocket::startServer(int portno){
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;


     //sockfd = socket(AF_INET, SOCK_STREAM, 0);
     sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd == SOCKET_ERROR || newsockfd < 0)
          error("ERROR on accept");
     connected = true;

     std::cout << "\n Mergetool connected\n";

     //std::thread t1(fetchData, newsockfd);
     //t1.join();

     /*
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     */
}

void MergeToolSocket::error(const char *msg)
{
    cout<<"Error^^\n";
    perror(msg);
    //exit(1);
}

void MergeToolSocket::setModel(SimulationModel* model)
{
	this->model = model;
	this->calcAnglesInited = false;
}
int num = 0;
void MergeToolSocket::update()
{
    if(connected && num % 20 == 0){
        receive();
        sendModellData();
        num = 1;
    }
    num++;
}

double MergeToolSocket::calcAngle(int otherShipID)
{
    double otherX = model->getOtherShipPosX(otherShipID);
	double otherZ = model->getOtherShipPosZ(otherShipID);

	double ownX = model->getPosX();
	double ownZ = model->getPosZ();

	double deltaX = otherX - ownX;
	double deltaZ = otherZ - ownZ;

    //cout<<"\notherX : "<< otherX << " otherZ " << otherZ<<",myX : "<< ownX << " myZ " << ownZ<< "ship: " << otherShipID;
    //cout<<"\ndX : "<< deltaX << " dZ " << deltaZ<< "ship: " << otherShipID;
    double angle = atan2(deltaZ, deltaX) * 180.0 / PI;
    angle = fmod(angle + 360.0 , 360.0); //between 0 and 360 instead -180 and 180
	return angle;
}

// TODO: Works only on rather straight maps! Will fail on curvy water streets!
bool MergeToolSocket::isShipPassed(double initShipAngle, double currentShipAngle){
    //cout<<"init angl: " << initShipAngle;
    double anglediff = fmod((initShipAngle - currentShipAngle + 360), 360);
    //anglediff = fmod(anglediff + 360.0 , 360.0); //between 0 and 360 instead -180 and 180
    // angle diff to init is at least 130
	return anglediff > 130.0 && anglediff < 230.0;
}

void MergeToolSocket::sendModellData()
{
    cout<<"anfang sende^^";
	/* type: irr::f32
	model->getLat()
	model->getLong()
	model->getCOG() //FIXME: currently the same as getHeading(), because current is not implemented, yet
	model->getSOG() //FIXME: currently the same as getSpeed(), because current is not implemented, yet
	model->getSpeed() // speed through water
	model->getHeading() // compass course
	model->getRudder()
	model->getRateOfTurn()
	model->getPortEngineRPM()
	model->getStbdEngineRPM()
	model->getPortEngine() //FIXME: currently commented out in SimulationModel
	model->getStbdEngine() //FIXME: currently commented out in SimulationModel
	*/

	double angleOtherShip1 = calcAngle(0);
	double angleOtherShip2 = calcAngle(1);
    bool ship1Passed = this->isShipPassed(this->otherShip1InitAngle, angleOtherShip1);
    bool ship2Passed = this->isShipPassed(this->otherShip2InitAngle, angleOtherShip2);

	if(!this->calcAnglesInited){
        this->calcAnglesInited= true;
        this->otherShip1InitAngle = angleOtherShip1;
        this->otherShip2InitAngle = angleOtherShip2;
	}
    //cout<<"\nship1 : "<< angleOtherShip1 << " ship 2: " << angleOtherShip2;
    //cout<<"ship1 : "<< ship1Passed << " ship 2: " << ship2Passed;



    double time = model->getSimulationTime();

    double tcpaShip1 = SHIP1_START_TIME - time;
    double tcpaShip2 = SHIP2_START_TIME - time;

    tcpaShip1 = tcpaShip1 / (60.0 * 1000.0); //from millisecs to minutes
    tcpaShip2 = tcpaShip2 / (60.0 * 1000.0); //from millisecs to minutes
    //cout<<"sim time: "<< time;

    /*struct timeval tv;
    gettimeofday(&tv, 0);
    long int ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    */

    std::string msg = ""; // + CMD_UPDATE_ENV_ELEM + to_string(ms) + ";";


    //ee_WaterSupply_long,8.187772,DOUBLE,;

    msg+= "ie_own_ship_course_og," + to_string(model->getCOG()) + ",STRING,;";
    msg+= "ie_own_ship_speed_og," + to_string(model->getSOG()) + ",DOUBLE,;";
    //msg+= "ie_own_ship_name," + to_string(model->getLat()) + ",STRING,;";
    //msg+= "ie_own_ship_type," + to_string(model->getLat()) + ";";

    msg+= "ie_own_ship_turn_rate," + to_string(model->getRateOfTurn()) + ",DOUBLE,;";
    msg+= "ie_own_ship_rudder_angle," + to_string(model->getRudder()) + ",DOUBLE,;";
    msg+= "ie_own_ship_orientation," + to_string(model->getHeading()) + ",DOUBLE,;";

    msg+= "ie_ship1_tcpa," + to_string(tcpaShip1) + ",DOUBLE,;";
    msg+= "ie_ship2_tcpa," + to_string(tcpaShip2) + ",DOUBLE,;";

    msg+= "ie_ship1_passed," + to_string(ship1Passed) + ",STRING,;";
    msg+= "ie_ship2_passed," + to_string(ship2Passed) + ",STRING,;";


/*for number to getNumberOfOtherShips():
	getOtherShipName(int number)
	getOtherShipPosX(int number) // convert to lat
	getOtherShipPosZ(int number) // convert to long
	getOtherShipHeading(int number)
	getOtherShipSpeed(int number)
	getOtherShipName
end for*/
    int numOtherShips = model->getNumberOfOtherShips();

/*		<infElem name="ie_ship1_course_og" dataType="STRING" />
		<infElem name="ie_ship1_speed_og" dataType="DOUBLE" />
		<infElem name="ie_ship1_name" dataType="STRING" />
		<infElem name="ie_ship1_tcpa" dataType="DOUBLE" />  <!-- time to closest point of Approach -->
*/

    for(int i=0; i<numOtherShips; i++){
	string prefix = "ie_ship" + to_string(i);
    	msg+= prefix + "_name," + model->getOtherShipName(i) + ",STRING,;";
    	msg+= prefix + "_course_og," + to_string(model->getOtherShipSpeed(i)) + ",STRING,;";
    	msg+= prefix + "_speed_og," + to_string(model->getOtherShipSpeed(i))  + ",DOUBLE,;";
    	//msg+= prefix + "_tcpa;" + to_string(model->TODO(i) + ";"); // TODO
    }
    msg+="\n";
    SOCKET n = send(newsockfd, msg.c_str() , msg.size(), 0);


    if (n == SOCKET_ERROR || n < 0) error("eERROR writing to socket");
    //n = send(newsockfd, buffer , 99, 0);
    cout<<" ... gesendet^^\n";
}



void MergeToolSocket::receive()
{
	/* type: irr::f32
	model->setRudder(irr::f32 rudder)
	model->setPortEngine(irr::f32 port)
	model->setStbdEngine(irr::f32 stbd)
	*/
}

