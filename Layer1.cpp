#include "Headers.h"

#define MAXBUFLEN 10000
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
unordered_map<int,int> m;



class ServerSocket{
	public:
	int sockfd;
	struct sockaddr_in my_addr;
	/* connector’s address information */
	struct sockaddr_in their_addr;
	int addr_len, numbytes;
	char buf[10000];
	
	ServerSocket(){
		numbytes=0;
		my_addr.sin_family = AF_INET;			/* short, network byte order */
		my_addr.sin_port = htons(4001);			/* automatically fill with my IP */
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
				cout<<"\nReceiving Request...";
				
				string response="";
				if(v[0]=="RD"){
				
					cout<<"\nL1:The Read value is:"<<m[stoi(v[1])];
					response = response + to_string(m[stoi(v[1])]);			
				
				} else if (v[0]=="WR"){
				
					m[stoi(v[1])] = stoi(v[2]);
					cout<<"\nL1:The Written value is:"<<m[stoi(v[1])];
				
				} else {

					/* Drop the Request */
				}

				strcpy(buf, response.c_str());
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
	ServerSocket s;

	cout<<"\nThread 1 Spawned..";
	s.rec_request();		
}
int main(){
	
	/* Start Work Load Generation */
	
	thread th1(th1_func);
	th1.join();

	// system("rm workload");
	// system("g++ -std=c++11 WorkLoadGen.cpp -o workload");
	// 	if(system("./workload"))			{
	// 	 Successful spawn of process 
	// } else {
	// 	//cout<<"WorkLoadGen start failed !! Aborting run";
	// 	//exit(0);
	// }
	
    // DC1 Port - 3000
    // DC2 Port - 3001
    
    /* */

	return 0;
}