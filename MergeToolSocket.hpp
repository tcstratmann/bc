/*   Bridge Command 5.0 - MergeToolSocket Integration
     Copyright (C) 2016 Dierk Brauer
     dierk.brauer@uni-oldenburg.de
*/

#ifndef __MERGETOOLSOCKET_HPP_INCLUDED__
#define __MERGETOOLSOCKET_HPP_INCLUDED__

#include <string>

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


//Forward declarations
class SimulationModel;

class MergeToolSocket
{
public:
    MergeToolSocket();
    ~MergeToolSocket();

    void setModel(SimulationModel* model);
    void update();
    SOCKET newsockfd, sockfd;

private:
    const std::string CMD_UPDATE_ENV_ELEM = "UPDATE_ENV_ELEM";
    bool calcAnglesInited;
    //int newsockfd, sockfd; //liux
    SimulationModel* model;
    double otherShip1InitAngle;
    double otherShip2InitAngle;
    void sendModellData();
    double calcAngle(int otherShipID);
    bool isShipPassed(double initShipAngle, double currentShipAngle);
    void receive();
    bool startServer(int port);
    void error(const char *msg);
};

#endif
