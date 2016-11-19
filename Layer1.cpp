#include "Headers.h"
vector<string> splitString(string input, string delimiter)
  {
     vector<string> output;
     char *pch;
  
     char *str = strdup(input.c_str());
  
     pch = strtok (str, delimiter.c_str());
  
     while (pch != NULL)
     {
        output.push_back(pch);
  
        pch = strtok (NULL,  delimiter.c_str());
     }
  
     free(str);
  
     return output;

 }
mutex mtx; 
unordered_map<int,int> m;

class ClientSocket{
			public:
			int sockfd;
            /* connector’s address information */
			struct sockaddr_in their_addr;
			struct hostent *he;
			int numbytes,addr_len;
		    char buffer[10000];

		    ClientSocket(int Port,string ip){
		    	string ip ="localhost";
				if ((he = gethostbyname(ip.c_str())) == NULL) {
					cout<<"Client-gethostbyname() error"<<endl;;
					exit(1);
				}
				else {	
						cout<<"Client-gethostname() is OK"<<endl;
				}
				if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
					cout<<"Client-socket() error lol!"<<endl;
					exit(1);
				}
				else {
					cout<<"Client-socket() sockfd is OK..."<<endl;			/* host byte order */
				}

				their_addr.sin_family = AF_INET;							/* short, network byte order */
				addr_len = sizeof(struct sockaddr);
				cout<<"Using port: "<<Port<<endl;
				their_addr.sin_port = htons(Port);
				their_addr.sin_addr = *((struct in_addr *)he->h_addr);       /* zero the rest of the struct */
				memset(&(their_addr.sin_zero), '\0', 8);

			}
			string send_request(string str){

				strcpy(buffer, str.c_str());
				printf("\nsending --> %s",buffer);
				if((numbytes = sendto(sockfd, buffer, strlen(buffer), 0, 
				 	(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
					cout<<"Client-sendto() error !"<<endl;
					exit(1);
				}
				else {
					numbytes=recvfrom(sockfd, buffer, strlen(buffer), 0, 
					(struct sockaddr *)&their_addr, (socklen_t*)&addr_len);

					string result="";	    		
		    		buffer[numbytes] = '\0';
					result=result+string(buffer);

					cout<<"Response is..."<<result<<endl;
					cout<<"sent "<<numbytes<<" bytes to "<<inet_ntoa(their_addr.sin_addr)<<endl;
					return result;
				}
				return NULL;

			}
			~ClientSocket(){
		
			/* Closing UDP socket */

				if (close(sockfd) != 0)
					cout<<"Client-sockfd closing is failed!"<<endl;
				else
					cout<<"Client-sockfd successfully closed!"<<endl;
			}


};

class ServerSocket{
	public:
	int sockfd;
	struct sockaddr_in my_addr;
	/* connector’s address information */
	struct sockaddr_in their_addr;
	int addr_len, numbytes;
	char buf[10000];
	
	ServerSocket(int port_no){
		numbytes=0;
		my_addr.sin_family = AF_INET;			/* short, network byte order */
		my_addr.sin_port = htons(port_no);			/* automatically fill with my IP */
		my_addr.sin_addr.s_addr = INADDR_ANY;	/* zero the rest of the struct */
		memset(&(my_addr.sin_zero), '\0', 8);

		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			cout<<"Server-socket() sockfd error lol!"<<endl;
			exit(1);
		} else {
			cout<<"Server-socket() sockfd is OK.."<<endl;
		
		}
		
		if(::bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) <0)
		{
			cout<<"Server-bind() error !"<<endl;
			exit(1);
		} else {
			cout<<"Server-bind() is OK.."<<endl;
		}
		addr_len = sizeof(struct sockaddr);
	}
	void rec_request(){
    cout<<"\norder chcck 2";
	//while(1){	


	    while((numbytes=recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
				(struct sockaddr *)&their_addr, (socklen_t*)&addr_len))!=-1) {
			    string result="";	    		
	    		buf[numbytes] = '\0';
				result=result+string(buf);
				
				vector<string> v=splitString(result,"#");
				cout<<"\nReceiving Request..."<<result;
				
				//int _Lead_Port_DC1 = (stoi(v[1]) %2==0)?5200:6200;
				int _Lead_Port_DC1 = 5200;
				int _Lead_Port_DC2 = 6200;

				/* Even-numbered requests would go to DC1 and odd-numbered to DC2*/
				
//				string _ip_DC1 = config_read((stoi(v[1]) %2==0)? "DC1_LEAD_IP" : "DC2_LEAD_IP");
				string _ip_DC1 = config_read("DC1_LEAD_IP");
				string _ip_DC2 = config_read("DC2_LEAD_IP");

				ClientSocket c1(_Lead_Port_DC1,_ip_DC1);
				ClientSocket c2(_Lead_Port_DC2,_ip_DC2);

				string response_DC1="";
				string response_DC2="";

				if(v[0]=="RD"){

					/* Send Synchronous writes to Leaders in all the DataCenters */
					response_DC1 = response_DC1 + c1.send_request(result);		
					response_DC2 = response_DC2 + c2.send_request(result);		
					response_DC1 = response_DC1 + response_DC2;

					cout<<"\nL1:The Read value is:"<<response_DC1;
					/* Actually Forward the request to Leader */
					/*response = response + to_string(m[stoi(v[1])]);*/
				} else if (v[0]=="WR"){
					response_DC1 = response_DC1 + c1.send_request(result);		
					response_DC2 = response_DC2 + c2.send_request(result);		
					response_DC1 = response_DC1 + response_DC2;

					/*m[stoi(v[1])] = stoi(v[2]);*/
					cout<<"\nL1:The Written value is:"<<response_DC1;
				} else {
					/* Drop the Request */
				}

				strcpy(buf, response_DC1.c_str());
				numbytes = sendto(sockfd,buf, strlen(buf), 0,(struct sockaddr *)&their_addr,sizeof(struct sockaddr));
       				if (numbytes  < 0) cout<<"\nFailed to send";
			}
		//}

	}
	~ServerSocket(){

		if(close(sockfd) != 0)
			cout<<"Server-sockfd closing failed!"<<endl;
		else
			cout<<"Server-sockfd successfully closed!"<<endl;
	}
	
};


void th1_func(){
	string port = config_read("LAYER1_PORT")
	ServerSocket s(stoi(port));

	cout<<"\nL1: Thread 1 Spawned..";
	s.rec_request();		
}
int main(){
	/* Start Work Load Generation */
	thread th1(th1_func);
	th1.join();
    
/*	system("rm workload");
	system("g++ -std=c++11 WorkLoadGen.cpp -o workload");

	if(system("./workload")){
		 cout<<"Successful spawn of process ";
	} else {
		cout<<"WorkLoadGen start failed !! Aborting run";
	}
	
    DC1 Port - 5200
    DC2 Port - 6201
    
 */

	return 0;
}