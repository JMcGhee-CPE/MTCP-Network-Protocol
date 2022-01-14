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
#include <sys/wait.h>
#include <poll.h>

int main( int argc, char *argv[] )
{


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
  
  
  int pipes[4][2];
  
  pipe(pipes[0]); 
  pipe(pipes[1]);
  pipe(pipes[2]);
  pipe(pipes[3]);
  
  int pid = 0;
  int tcp_id = 0;
  
  
  
  if (tcp_id == 0){
  
     // Initial socket, master connection
    int sd = socket(AF_INET, SOCK_STREAM, 0);

 
    // Connect to port 1025 on server address
    // abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
    int err = connect( sd, (struct sockaddr*)&ReqAddr, sizeof(ReqAddr));
    
    char server_okay[20];
    
    read(sd, server_okay, sizeof(server_okay));  // read from the TCP connections that will send data
   
    if(err != 0) {
      printf("Could not connect to server! %s\n", strerror(errno));    
    }

  
    char send_data[1000] = { '\0' };
  
    printf("Please enter data to send: ");
      
    scanf("%s",send_data);
      
    printf("Sending %s\n", send_data);
    
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
  
  printf("Child process started\n");
  
      char server_id[10] = { '\0' };
      
      if(tcp_id > 0){
      
        ReqAddr.sin_port = htons(port + tcp_id);



        // Initial socket, master connection
        int child_socket = socket(AF_INET, SOCK_STREAM, 0);


        // Connect to port 1025 on server address
        int child_err = connect( child_socket, (struct sockaddr*)&ReqAddr, sizeof(ReqAddr));
        
        
        read(child_socket, server_id, sizeof(server_id));  // read from the TCP connections that will send data


        if(child_err != 0) {
            printf("Could not connect to server! %s\n", strerror(errno));    
        }

      
        while(tcp_id > 0){
        
                char stcp_id[10] = {'\0'};
                
                sprintf(stcp_id, "%d:%s|", tcp_id, server_id);
        
                write(pipes[0][1], stcp_id, 10); // Message parent that this is ready to send
        
                //printf("[child] - Reading on pipe %d\n", tcp_id);
        
                // Read data from parent process    

                char read_data[3];

                read(pipes[tcp_id][0], read_data, 2);
                
                read_data[2] = '\0';
                
                if(read_data[0] == '\x03'){ // Message from parent to close
                    printf("Closed child connection!\n");
                    shutdown(child_socket, SHUT_WR);
                    close(child_socket);
                    return;
                }
                
                //printf("[child] Read data! %s\n", read_data);
                
                
                // Send data via tcp to the server        

                write( child_socket, read_data, strlen(read_data));
                
                printf("[child] Send data %s\n", read_data);
                
                // Clear read buffer to prepare for next iteration
                
                memset(read_data,0,sizeof(read_data));
                

        }
      }
  
    int send_length = strlen(send_data);
  
    int str_idx = 0;
    
    FILE *fp;
    
    fp = fopen("client_log.txt","w");
  
      while (str_idx < send_length){
      
        printf("[Parent] - Listening for children\n");
        
        char next_child[11] = { '\0' };
        
        read(pipes[0][0], next_child, 10); // Get next child that is waiting for data
        
        char source_tcp[20] = {'\0'};
        char server_id[20] = {'\0'};
        
        //printf("Read from child %s\n", next_child);
        
        int i;
        
        for(i = 0; i < 10; i++){
            if(next_child[i] == ':'){
                break;
            }
            source_tcp[i] = next_child[i];
        }
        
        int offset = ++i;

        for(i = i; i < 20; i++){
            if(next_child[i] == '|'){
                break;
            }
            server_id[i - offset] = next_child[i];
        }
        
        char raw_data[3] = {'\0'};
        
        strncpy( raw_data, &send_data[str_idx], 2);
        
        int sub_proc_to_send = atoi(source_tcp);
        
        //printf("[Parent] - Sending %s(%d bytes) to server from tcp %d\n", raw_data, strlen(raw_data), sub_proc_to_send);
        
        
        write(pipes[sub_proc_to_send][1], raw_data, strlen(raw_data));
        
        //printf("data strlen %d\n", strlen(read_data));
        
        
        //printf("[Parent] - Finished reading tcp %s read %d chars! Server id = %s\n", source_tcp, strlen(raw_data), server_id);
        
        char buff[20] = { '\0' };
        
        sprintf(buff, "%3d %3d %3s\0", str_idx, strlen(raw_data), server_id);
        
        fprintf(fp, "%d => %s\n", str_idx, server_id);
        
        printf("Writing to server\n");
        
        write( sd, buff, strlen(buff));
        
        //printf("[Parent] - Sent %s to server!", buff);
        
        str_idx += strlen(raw_data);
      
      }
      
      printf("Finished reading!\n");    
      fclose(fp);
      
      write(pipes[1][1], "\x03", 1);
      write(pipes[2][1], "\x03", 1);
      write(pipes[3][1], "\x03", 1);
      
      //write( sd, "\x03", 1);
      
      //shutdown(sd, SHUT_WR);
      close(sd);
      
      
      
  }
  

  return 0;
}
