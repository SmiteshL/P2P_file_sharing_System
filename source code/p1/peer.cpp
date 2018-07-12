#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctime>
#include <sstream>

using namespace std;

socklen_t CSclien;
int sockfd, portno, portno2, n, choice, count, peer_list, CSsockfd,  CSnewsockfd, con_to,remain_data,fd,file_size,sockfd_serExt,tmp,z,y[10], sent_bytes = 0, listNumber =0,found=0;;
long int offset;

struct sockaddr_in serv_addr,serv2_addr,CSserv_addr, CScli_addr;
struct hostent *server;
char file_name[50],file_in[50], buffer[BUFSIZ],file_sz[50];
vector<string> peerVec(10);
struct stat file_stat;
 FILE *received_file;
ssize_t len;
fstream server_list;
vector<int> serv_list;
string line;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void getfile(int con_to,char *file_name ){
    int w=con_to;  
         
    CSsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (CSsockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &CSserv_addr, sizeof(CSserv_addr));
    CSserv_addr.sin_family = AF_INET;
    CSserv_addr.sin_addr.s_addr = INADDR_ANY;
    CSserv_addr.sin_port = htons(w);
    
    if(connect(CSsockfd,(struct sockaddr *) &CSserv_addr, sizeof(CSserv_addr)) < 0) 
        error("ERROR connecting to target peer");
     
    cout<<"connected to file source peer at: " <<con_to<<endl;
    
    n = send(CSsockfd,file_name,sizeof(file_name),0);
    if (n < 0) 
        error("ERROR writing filename to socket");
    int start_s=clock();
    n= recv(CSsockfd, &w, sizeof(w), 0);
    if (n < 0) 
           error("ERROR receiving filesize to socket");
    file_size = ntohl(w);
    
   // cout<<"size of file to download : " <<file_size<<endl;
    
    received_file = fopen(file_name, "w");
    if (received_file == NULL){
        fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    remain_data = file_size;

    while (remain_data > 0){
        len = recv(CSsockfd, buffer, BUFSIZ, 0);
        fwrite(buffer, sizeof(char), len, received_file);
        remain_data -= len;
    }
    int stop_s=clock();
    
    
    fclose(received_file);
    
    cout<<"file transfer complete"<<endl;
    cout<<file_name<<" of size "<<file_size<<" bytes "<<" downloaded in "<<(stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 <<" seconds!"<<endl;  
    
    close(CSsockfd);
    return;
}


void nextServerSearch(int serPort){
    
    listNumber++;   // increment visited server
    serv2_addr.sin_port = htons(serPort);//use connect to next server in list
    
    
    n = connect(sockfd_serExt,(struct sockaddr *) &serv2_addr, sizeof(serv2_addr));
    if (n < 0)
        error("ERROR connecting to another server");
            
     
    //cout<<"flagis"<<flag<<endl;
    cout<<"connected to server at: "<<serPort<<endl;
        
    n = send(sockfd_serExt,"2",1,0);
    if (n < 0) 
        error("ERROR writing to socket");
    
    n = send(sockfd_serExt,file_name,sizeof(file_name),0);
    if (n < 0) 
           error("ERROR writing filename to socket");    
    
    
    n = recv(sockfd_serExt, &tmp, sizeof(tmp),0);//number of results in externalserver
    if (n < 0) 
        error("ERROR getting peercount to socket");
    
    count = ntohl(tmp);
    
        // if count =0 connect to next server else 
    if (count==0){
        cout<<"file not found on this server "<<endl;
        
            return;
    } 
    else{
        //found on second server server
        // close sockfd_serExt and print available peer numbers
        found=1;
        
        cout<<"file available on ports: "<<endl;
        
        for(tmp=0;tmp<count;tmp++){   
            
            n = recv(sockfd_serExt, &peer_list, sizeof(peer_list),0);
            if (n < 0) 
                error("ERROR getting peercount to socket");

            cout<<ntohl(peer_list)<<endl; 
        }
    }
    
    cout<<"enter peer number to download file or 0 to exit."<<endl;
    
    cin>>z;
    
    if (z != 0)
        getfile(z,file_name);    

    
}




void external_search_req(){
    
    // initialise serv2_addr,sockfd_serExt
    while (listNumber < serv_list.size()){
        
        
    sockfd_serExt = socket(AF_INET, SOCK_STREAM, 0);//socket for connecting to other server
    if (sockfd_serExt < 0)
        error("ERROR opening socket");     
    
    int enable = 1;//enable address reuse
    if (setsockopt(sockfd_serExt, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed");    
    
    //modify to read from serverlist
    
    //cout<<"listNumber"<<listNumber<<endl;
        
        nextServerSearch(serv_list[listNumber]);
        if ((z == 0)&&(found==1))
                break;
        close(sockfd_serExt);
    }
    
    
listNumber = 0;
found=0;
}



void index_file(){
    int tmp2 = htonl(portno2);
    n = send(sockfd,&tmp2,sizeof(tmp2),0);// 
    if (n < 0) 
        error("ERROR writing port number of client/peer to socket");

    cout<< "enter file name to register :"<<endl;
    cin>>file_name;
    
    n = send(sockfd,file_name,sizeof(file_name),0);
    if (n < 0) 
           error("ERROR writing filename to socket");
    
    cout<< file_name <<" registered with server"<< endl;
        
}

void lookup_file(){
    
    
    cout<< "enter file name to search:"<<endl;
    cin>>file_name;
    
    n = send(sockfd,file_name,sizeof(file_name),0);
    if (n < 0) 
           error("ERROR writing filename to socket");
        
    n = recv(sockfd, &tmp, sizeof(tmp),0);
    if (n < 0) 
        error("ERROR getting peercount to socket");
    
    count = ntohl(tmp);
        
    
    
    if (count==0){
        cout<<"file not found on this original server. connecting to other servers: "<<endl;
        
        //call external search thread
        thread tex{external_search_req};
        tex.join();
        
        
        
        
        
        return;
    } 
    else{
        //found on native server
        cout<<"file available on ports: "<<endl;
        
        for(tmp=0;tmp<count;tmp++)
        {   
            //bzero(peer_list,32);
            n = recv(sockfd, &peer_list, sizeof(peer_list),0);
            if (n < 0) 
                error("ERROR getting peercount to socket");

            
    
                cout<<ntohl(peer_list)<<endl; 
        }
    }
    
    cout<<"enter peer number to download file or 0 to exit."<<endl;
    
    cin>>z;
    
    if (z != 0)
        getfile(z,file_name);
    
 //   cout<<"rturned from getfie"<<endl;
        
}

void clientServerThread(int port){
    
    cout<<"port to act as server for this Peer :>"<<port<<endl;
    
    CSsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (CSsockfd < 0)
         error("ERROR opening socket");
    bzero((char *) &CSserv_addr, sizeof(CSserv_addr));
   
    CSserv_addr.sin_family = AF_INET;
    CSserv_addr.sin_addr.s_addr = INADDR_ANY;
    CSserv_addr.sin_port = htons(port);
    if (bind(CSsockfd, (struct sockaddr *) &CSserv_addr,sizeof(CSserv_addr)) < 0)
        error("ERROR on binding");

    listen(CSsockfd, 5);
    CSclien = sizeof(CScli_addr);
    
    int pidd;
    while (1) {
         CSnewsockfd = accept(CSsockfd, (struct sockaddr *) &CScli_addr, &CSclien);
         if (CSnewsockfd < 0)
                error("ERROR on accept");
         
          pidd = fork();
         
         if (pidd < 0) {
              error("ERROR in new process creation");
         }
         if (pidd == 0) {
   
        cout<<"Client server connected to:" <<CScli_addr.sin_addr.s_addr<< ":" << ntohl(CScli_addr.sin_port) << endl;
        
        bzero(file_in,50);
        n = recv(CSnewsockfd, file_in, sizeof(file_in),0);// read file name
        if (n < 0)
            error("ERROR reading from socket"); 
        
        cout<< "requested file: "<< file_in<<endl;
        
        fd = open(file_in, O_RDONLY);
        if (fd == -1)
        {
                fprintf(stderr, "Error opening file --> %s", strerror(errno));

        }
        
        if (fstat(fd, &file_stat) < 0)
                fprintf(stderr, "Error fstat --> %s", strerror(errno));
        

        fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);

        int tmp = htonl(file_stat.st_size);
        
        cout<< file_stat.st_size<<"\t"<<file_sz<<endl;
        n = send(CSnewsockfd, &tmp, sizeof(tmp), 0);
            if (n < 0)
        {
              fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

              exit(EXIT_FAILURE);
        }


        offset = 0;
        remain_data = file_stat.st_size;
        
        /* Sending file data */
        while (((sent_bytes = sendfile(CSnewsockfd, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
        {

        }
        
        close(CSnewsockfd);
    
         }

    }
 
}

void get_serverlist(){
    int a;
    
    server_list.open ("server_list.txt");
    
    ifstream infile("server_list.txt");
    
    while (getline(infile, line)){
        istringstream iss(line);
        if (!(iss >> a )) { break; } // error
                //cout<<a<<endl;
                serv_list .push_back(a);
            
    }

    server_list.close();//close file

}



int main(int argc, char *argv[])
{
    portno2 = atoi(argv[2]);//port no when client acts as server, bind here
    thread t1{clientServerThread, portno2};
    t1.detach();
    
    thread get_slist{get_serverlist};
    get_slist.detach();
    
    
    if (argc < 3) {
      fprintf(stderr, "ERROR, format: ./peer [server port no.] [client port no.]\n");
       
      exit(0);
    }
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    serv2_addr = serv_addr;
    
    if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    while(1){
        
        
      
    cout<<"\nChoose action: \n1.index existing file\n2.serch file on servers\n"<<endl;
    cin >>choice;
    
    
    
    
    int x= choice;
    switch (choice)   {
        case (1): n = send(sockfd,"1",1,0);
                if (n < 0) 
                error("ERROR writing to socket");
                index_file();
                break;
        case (2): n = send(sockfd,"2",1,0);
                if (n < 0) 
                error("ERROR writing to socket");
                lookup_file();
                break;   
                
        default:cout<<"\nselect proper choice"<<endl;
        
    }
    }

    return 0;
}
