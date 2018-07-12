#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <ctime>

using namespace std;

int sockfd, newsockfd, portno, mmode, n, peer_port_number;
socklen_t clilen;
char buffer[256], mode[1], file_name[50];
struct sockaddr_in serv_addr, cli_addr;
fstream file_registry;
vector<int> resultVec;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void update_registry(){
    
        int tmp;
        n = recv(newsockfd, &tmp, sizeof(tmp),0);// read for peer id number
            peer_port_number = ntohl(tmp);
        if (n < 0)
            error("ERROR reading from socket"); 
    
        cout<< peer_port_number<<endl;      
        file_registry << peer_port_number<<"\t";
  
        bzero(file_name,50);
        n = recv(newsockfd, file_name, sizeof(file_name),0);// read file name
        if (n < 0)
            error("ERROR reading from socket"); 
        
        cout<< file_name<<endl;
        file_registry << file_name<<endl;  
}

vector<int> search_registry(){
    string  b, line;
    int a,z;
    
    vector<int> residual;
    residual.clear();
    
    bzero(file_name,50);
    n = recv(newsockfd, file_name, sizeof(file_name),0);// read file name to search
    if (n < 0)
        error("ERROR reading from socket"); 
    
    cout<<"Seraching:  "<< file_name<<endl;   
    
    ifstream infile("file_registry.txt");
    
    while (getline(infile, line)){
        istringstream iss(line);
        if (!(iss >> a >> b)) { break; } // error
            if (file_name == b) {
                cout<<a<<endl;
                residual .push_back(a);
            }
            
        }
        int tmp = htonl(residual.size());
        
        n=  send(newsockfd, &tmp, sizeof(tmp),0);
        if (n < 0) 
            error("ERROR writing peer Count to socket"); 
        
        
        
        for(tmp=0;tmp<residual.size();tmp++){
            z=htonl(residual[tmp]);
            n=  send(newsockfd, &z,sizeof(int) ,0);
            if (n < 0) 
                error("ERROR writing peer Count to socket");    
        }
        
        
    return residual;
        
}

int main(int argc, char *argv[]) {
    file_registry.open ("file_registry.txt");
        
    if (argc < 2) {
        fprintf(stderr,"usage %s  port\n", argv[0]);
         exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
         error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    cout<<"=====================================================" << endl;
    cout<<"           Indexing server sratrted" << endl;
    cout<<"=====================================================" << endl;
    
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    int pid;
    while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0)
                error("ERROR on accept");
         
         pid = fork();
         
         if (pid < 0) {
              error("ERROR in new process creation");
         }
         if (pid == 0) {
            //child process
   
        cout<<"server connected to port  : " << ntohs(cli_addr.sin_port) << endl;
            
        while(1){
            
        bzero(mode,1);
        n = recv(newsockfd, mode, 1,0);// read for client mode
        if (n < 0)
            error("ERROR reading from socket");                  
        
        mmode = *mode - '0';
        
        //cout<<mmode<<endl; 
        switch (mmode)   {
            case (1):
                    cout<<"server updating registry index"<<endl;
                    update_registry();
                    break;
            case (2):{
                    cout<<"server searching in registry index\n"<<endl;
                    resultVec =search_registry();
                    break;                
                        
                }
            //default:cout<<"\ninvalid responce from client"<<endl;
            
        }
        }//case while 
        }//if pid
    }
    
    file_registry.close();
   return 0;
}
