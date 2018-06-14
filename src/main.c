// Client-side UDP Code 
// Written by Sarvesh Kulkarni <sarvesh.kulkarni@villanova.edu>
// Adapted for use from "Beej's Guide to Network Programming" (C) 2017


#include <stdio.h>		// Std i/o libraries - obviously
#include <stdlib.h>		// Std C library for utility fns & NULL defn
#include <unistd.h>		// System calls, also declares STDOUT_FILENO
#include <errno.h>	    // Pre-defined C error codes (in Linux)
#include <string.h>		// String operations - surely, you know this!
#include <sys/types.h>  // Defns of system data types
#include <sys/socket.h> // Unix Socket libraries for TCP, UDP, etc.
#include <netinet/in.h> // INET constants
#include <arpa/inet.h>  // Conversion of IP addresses, etc.
#include <netdb.h>		// Network database operations, incl. getaddrinfo
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>



// Our constants ..
#define MAXBUF 10000     // 4K max buffer size for i/o over nwk
#define SRVR_PORT "5555"  // the server's port# to which we send data
						  // NOTE: ports 0 -1023 are reserved for superuser!

//checksum function
//takes in a character array and count
//returns checksum in uint16_t
uint16_t chksum(uint16_t *buf, int count)
	{
	register unsigned long sum = 0;

	while (count>0)
	{
	sum += *buf++;
	if (sum & 0xFFFF0000)
	{
	/* carry occurred,
	so wrap around */
	sum &= 0xFFFF;
	sum++;
	}
		count = count - 2; 
	}
	return ~(sum & 0xFFFF);
	
	}


int main(int argc, char *argv[]) {
	

  
    int sockfd;             // Socket file descriptor; much like a file descriptor
    struct addrinfo hints, *servinfo, *p; // Address structure and ptrs to them
    int rv, nbytes, nread;
    char buf[MAXBUF];    // Size of our network app i/o buffer

	
    
    if (argc != 3) {
        fprintf(stderr,"ERROR! Correct Usage is: ./program_name server userid\n"
                "Where,\n    server = server_name or ip_address, and\n"
                "    userid = your LDAP (VU) userid\n");
        exit(1);
    }

	

    // First, we need to fill out some fields of the 'hints' struct
    memset(&hints, 0, sizeof hints); // fill zeroes in the hints struc
    hints.ai_family = AF_UNSPEC;     // AF_UNSPEC means IPv4 or IPv6; don't care
    hints.ai_socktype = SOCK_DGRAM;  // SOCK_DGRAM means UDP

    // Then, we call getaddrinfo() to fill out other fields of the struct 'hints
    // automagically for us; servinfo will now point to the addrinfo structure
    // of course, if getaddrinfo() fails to execute correctly, it will report an
    // error in the return value (rv). rv=0 implies no error. If we do get an
    // error, then the function gai_strerror() will print it out for us
    if ((rv = getaddrinfo(argv[1], SRVR_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // We start by pointing p to wherever servinfo is pointing; this could be
    // the very start of a linked list of addrinfo structs. So, try every one of
    // them, and open a socket with the very first one that allows us to
    // Note that if a socket() call fails (i.e. if it returns -1), we continue
    // to try opening a socket by advancing to the next node of the list
    // by means of the stmt: p = p->ai_next (ai_next is the next ptr, defined in
    // struct addrinfo).
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("CLIENT: socket");
            continue;
        }

        break;
    }

    // OK, keep calm - if p==NULL, then it means that we cycled through whole
    // linked list, but did not manage to open a socket! We have failed, and
    // with a deep hearted sigh, accept defeat; with our tail between our legs,
    // we terminate our program here with error code 2 (from main).
    if (p == NULL) {
        fprintf(stderr, "CLIENT: failed to create socket\n");
        return 2;
    }

    // If p!=NULL, then things are looking up; the OS has opened a socket for
    // us and given us a socket descriptor. We are cleared to send! Hurray!
    // The sendto() function will report how many bytes (nbytes) it sent; but a
    // negative value (-1) means failure. Sighhh. 
    if ((nbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("CLIENT: sendto");
        exit(1);
    }

	
    printf("CLIENT: sent '%s' (%d bytes) to %s\n", argv[2], nbytes, argv[1]);
   
	// Recv packet from server. YOu should modify this part of the program so that
	// you can receive more than 1 packet from the server. In fact, you should
	// call recvfrom() repeatedly till all parts of the file have been received.

	//declare variables for the filename, path, size of packet, int for size of packet
	//int for total number of packets, checksums, and  array to hold each packet size
	char filename[MAXBUF];
	char path[MAXBUF];
	char size[4];
	int sizeval = 0;
	int numFiles = 0;
	char checksum[4];
	uint16_t checksumval = 0;
	uint16_t checksumval2 = 0;
	char packSize[7];

	
	//first read in from server containing packet name and size
	nread = recvfrom(sockfd,buf,MAXBUF,0,NULL,NULL);
	
	if (nread<0) {
		perror("CLIENT: Problem in recvfrom");
		exit(1);
	}

	//put file size from first packet into the size array and convert it to int
	for(int i = 0; i < 4; i++){
		size[i] = buf[i];
		checksum[i] = buf[i+4];
	}

	sizeval = atoi(size);

	//create unsigned int to hold checksum and convert it from string to ul
	uint16_t chcksum = 0;
	chcksum = strtoul(checksum,NULL,16); 
	checksumval = atoi(checksum);
	
    //print checksum for testing 
	printf("%#04x\n", chcksum);

    //get file name from first packet and put it into array for process
	for(int i = 0; i < MAXBUF - 8; i++){
		filename[i] = buf[i+8];
    }
	
	//add filename to path name to fetch file or create new file from name if file is not in directory 
	strcpy(path,"data_files/");
	strcat(path,filename);
	
    //open file to write to
	FILE * fp;
    fp = fopen(path,"w");

	//find number of files based on known payload size
	//if number is no divisible by 100, round up to make sure number of files is correct
	numFiles = sizeval / 100;

	if((sizeval % 100)!=0){
		numFiles++;
	}

	//create poem array based on number of files and payload size
	//packNum will hold the current packet number, val will be the int value
	char poem [numFiles][100];
	char packNum [2];
	int val;
	int lastcount = 0;
	int lastindex = 0;
		 

	//loop for the number of files in total
	for(int i = 0; i < numFiles; i++){
	
		//read in first packet
		nread = recvfrom(sockfd,buf,MAXBUF,0,NULL,NULL);
		if (nread<0) {
		perror("CLIENT: Problem in recvfrom");
		exit(1);
		}

		//set packNum to correct packet number from buf
		//packSize is nread - 6 or 100 
		//use atoi to set val to int form from packNum
		packNum[0] = buf[0];
		packNum[1] = buf[1];
		packSize[i] = nread-6;

		val = atoi(packNum);
	
	
		for(int j = 0; j < nread-6; j++){
	
			poem[val][j] = buf[j + 6];
			lastindex = j;
			
		}
			lastcount += nread-6;
	}
	
	//add blank character if array size is odd
	printf("%d\n",lastcount);
	if(lastcount%2 != 0)
	{
		poem[numFiles][lastindex] = '\0';
		lastcount++;
		
	}

	//run checksum function on poem and check if it equals sent checksum
	//if equal write poem to file 
	checksumval2 = chksum(poem,lastcount);
	
	printf("%#04x\n",checksumval2);
	
	if(checksumval2 == chcksum){
		printf("%s\n","success");
		


	// AFTER all packets have been received ....

	//print to the file (fp) 
	for(int i = 0; i < numFiles; i++){
		if (write(fileno(fp),poem[i], packSize[i]) < 0) {
			perror("CLIENT: Problem writing to stdout");
			exit(1);
		}
	}
			}
	//else prompt user for name and resend
	else{ 
		char name[20];
		char choice;
	    printf("%s\n","failure");
		printf("Countinue? [y/n]\n");
		scanf("%s",&choice);
		if(choice== 'y'){
		printf("\nResubmit username: \n");
		scanf("%s", &name);	
		argv[2] = name;
			return main(argc, argv);
		
		}
	}
	//printf("%#04x",checksumval2);
		
	// free up the linked-list memory that was allocated for us so graciously
	// getaddrinfo() above; and close the socket as well - otherwise, bad things
	// could happen
    freeaddrinfo(servinfo);
	if(fp == NULL){
		printf("%s\n","failed to open file");
	}
	else
	fclose(fp);
	  //printf("%d\n",2);
    close(sockfd);
  
	printf("\n\n"); // So that the new terminal prompt starts two lines below
	
    return 0;
}