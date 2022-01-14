
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>



int main( int argc, char *argv[] )
{


  char print_data[1000];


  //for testing, initially, all elements in print_data array will be '0'
  int i;

  for( i = 0; i < 1000; i++){

     print_data[i] = 0;

  }



  int port = 5000;

  // Setup initial socket *normally port 21, but to avoid needing root it is 1025

  struct sockaddr_in ResponseAddr;
  int RecAddrLen;

  // Setting up tcp socket --------
  
  
  struct sockaddr_in ReqAddr = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
    .sin_addr = {
        .s_addr = inet_addr( argv[1] ) // Passed in from arguements
    }
  };
  
  RecAddrLen = sizeof(ReqAddr);


 struct sockaddr_in ClientReqAddr, ClientDataAddr;  




  // Initial socket, master connection
  int sd = socket(AF_INET, SOCK_STREAM, 0);

 
  

 //Binding
  
  int err_master = bind(sd, (struct sockaddr *) &ReqAddr, sizeof(ReqAddr));
      
  if(err_master != 0) {
    printf("Could not bind RecieveAddr port! %s\n", strerror(errno));  
    return -1;  
  }

 //listen
 listen(sd, 1);
  
   
 //accepting the connection
 int reqSocket = accept(sd, (struct sockaddr *)&ReqAddr, &RecAddrLen);
  
  int pipes[3][2];
  
  pipe(pipes[0]); 
  pipe(pipes[1]);
  pipe(pipes[2]);
  
  
  int pid;
  int tcp_id = 0;
  if (pid = fork()){
    if(pid = fork()){
      if(pid = fork()){
            tcp_id = 3;
      } else {
          tcp_id = 2;
      }    
    } else {
        tcp_id = 1;
    }
  }
  
  if(tcp_id != 0){  // in a child process

    printf("Binding to port %d\n", port + tcp_id);  
  
    ReqAddr.sin_port = htons(port + tcp_id);


    // Initial socket, child connection
    int child_socket = socket(AF_INET, SOCK_STREAM, 0);


    // Connect to port 1025 on server address
    //int child_err = connect(child_socket, (struct sockaddr*)&ReqAddr, sizeof(ReqAddr));

    //Binding

    int err = bind(child_socket, (struct sockaddr *) &ReqAddr, RecAddrLen);
      
    if(err != 0) {
      printf("Could not bind RecieveAddr port! %s\n", strerror(errno));  
      return -1;  
    }

    //listen
    
    
      int listenRes = listen(child_socket, 1);

      if (listenRes < 0){
        
        perror("There was an error listening to socket!");

      }
  
   
    //accepting the connection
    int dataSocket = accept(child_socket, (struct sockaddr *)&ReqAddr, &RecAddrLen);
    
    printf("Recieved Connection!\n");
    
    char connection_id[3] = { 0 };
    
    sprintf(connection_id, "%d", tcp_id, 2);
    
    write( dataSocket, connection_id, strlen(connection_id));
    
  
    while(tcp_id != 0){ // infinite loop

	char data_from_client[30];
	
	memset(data_from_client,0,30);

    int num_read = read(dataSocket, data_from_client, sizeof(data_from_client));  // read from the TCP connections that will send data
    
    if(num_read == 0){ // Message from client to close
        printf("Closed child connection!\n");
        close(dataSocket);
        return;
    }
    
    int lastIdx = strlen(data_from_client);
    
    
    printf("[Socket Read] Sending data to parent %s, from tcp connection %d\n", data_from_client, tcp_id);

	
	/* piping the message to the parent process*/
	
	     write(pipes[tcp_id - 1][1], data_from_client, strlen(data_from_client));
        
         printf("Received data %s\n", data_from_client); // just for testing

    }
  }


  else{  // if inside the parent process
  
    write( reqSocket, "okay", strlen("okay"));    
    
    FILE *fp;
    
    fp = fopen("server_log.txt","w");
    
    int numRead = 0;

	while( numRead < 992 ){ // reading the TCP Master data and then sending them to a child process using pipe()

	     char masterData[12];  // will hold the message from the master TCP
	     
	     memset(masterData,0,12);

	     read(reqSocket, masterData, 11);  // reading the message from the master TCP from the client



	     /* will convert "index size connection" -> {index, size, connection}  */

	     int array_of_ints[3]; // {index, size, connection}

	     int z = 0; // to iterate through array_of_ints[] values

	     char space[] = " ";
	     
	     printf("[Socket Read] Recieved message from client %s\n", masterData);
	     
	     char recv_vals[3][4] = {0};
	     
	     for( z = 0; z < 3; z++ ){
	        int y;
	        for(y = 0; y < 3; y++){
	            if( masterData[ (3*z) + y + z] == ' ' ){
	                recv_vals[z][y] = '0';
	            } else {
	                recv_vals[z][y] = masterData[ (3*z) + y + z];
	            }
	        }
	        //printf("Raw string %s\n", recv_vals[z]);
	        array_of_ints[z] = atoi(recv_vals[z]);
	        //printf("Read int %d, at idx %d\n", array_of_ints[z], z);
	        
	     }


	     /***********************/


	/*Receiving the data from one of the child processes */
        char read_data[1];
         
        int x = array_of_ints[0];
	
	    int counter = 0;
	    for(counter = x; counter < x + array_of_ints[1]; counter++){
    	    read(pipes[array_of_ints[2] - 1][0], read_data, 1);  // read the message from one of the child processes ex: "abc"
		    print_data[counter] = read_data[0];
		    //printf("Character read %c\n", read_data[0]);
	    }
	    numRead += array_of_ints[1];
	    fprintf(fp, "%d => %d\n", array_of_ints[2], array_of_ints[0]);
	    printf("Num read - %d\n", numRead);
	    //read(pipes[array_of_ints[2] - 1][0], read_data, 1); // Remove trailing whitespace
	    
	    //printf("Print Data - %s\n", print_data);

	/*At this point I have the message ex: "abc", and an array of {index, size, connection}, which will be used to set the    appropriate indexes in the print_data string*/


	
	//printf("\nprint_data: %s\n", print_data);   // for testing
	  

    }
    
    printf("Finished reading\n");

    fclose(fp);


  }
  
  

  return 0;
}

