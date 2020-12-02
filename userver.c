/*
    Simple udp server
*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define BUFLEN 1024  //Max length of buffer

//print error and exit
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other;                  //socket struct
    int cSum = 0, carry = 0, tmp = 0;                    //for calculating checksum
    unsigned short int cSumShort = 0;                    //stores calculated checksum
    int sockfd, i, slen = sizeof(si_other) , recv_len;   //socket variables
    char buf[BUFLEN];                                    //send/recv msg buffer
    FILE *fp;                                            //output file pointer

    //received UDP segment struct
    struct recvSegment {
        unsigned short int sPort;
        unsigned short int dPort;
        unsigned short int len;
        unsigned short int chksum;
        char msgData[BUFLEN];
    } rSeg;

    //memset struct to 0
    memset(&rSeg, 0, sizeof(rSeg));

    //check for correct usage
    if (argc != 2)
	{
		die("Usage: ./userver <portnumber>\n");
	}
     
    //create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    //set socket struct data
    si_me.sin_family = AF_INET;
    si_me.sin_port = atoi(argv[1]);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket
    if(bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    //receive empty message from client
    fflush(stdout);
    bzero (buf, BUFLEN);
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
    {
        die("recvfrom()");
    }

    //sprintf source port into buffer
    bzero (buf, BUFLEN);
    sprintf(buf, "%d", ntohs(si_other.sin_port));
  
    //send source port to client
    if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
    {
        die("sendto()");
    }

    printf("Waiting for segment...\n");
    fflush(stdout);
    bzero (buf, BUFLEN); 
    //receive UDP segment from client
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
    {
        die("recvfrom()");
    }

    //memcpy buffer into server segment struct
    memcpy(&rSeg, buf, recv_len);

    //calculate n, the number of elements needed in the checksum array, then create array
    int n = ((rSeg.len * 8)/16) + 1;  //same as (rSeg.len + 1)/2;
    unsigned short int chkArr[n];

    //store the client's checksum for comparison to server-calculated checksum later
    unsigned short int tmpChk = rSeg.chksum;

    //zero out client-sent chksum before calculating server checksum
    bzero(&rSeg.chksum, sizeof(rSeg.chksum));

    //memcpy contents of received struct into array
    memcpy(chkArr, &rSeg, rSeg.len);

    //for each element in the array...
    for (int i = 0; i < n; i++)
    {
        //add current array element to 32bit cSum
        cSum += chkArr[i];

        //copy sum into tmp, zeroing out bits 17-32 with bitwise &
        tmp = tmp & cSum;

        //shift cSum 16 bits to the right to get carry (if any)
        carry = (cSum)>>16;

        //add (tmp+carry) to 16 bit cSumShort
        cSumShort += (tmp+carry);
    }

    //if checksum values match...
    if (cSumShort == tmpChk)
    {
        printf("Checksum verified, writing payload to server.out...\n");
        
        //store calculated checksum
        rSeg.chksum = cSumShort;

        //write payload data sent by client to s.out
        char ch;
        fp = fopen("server.out", "w");
        for (int i = 0; i<strlen(rSeg.msgData); i++)
        {
            ch = rSeg.msgData[i];
            fputc(ch, fp);
        }
        fclose(fp);

        //let client know checksum was verified
        if (sendto(sockfd, "success", strlen("success"), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }

        printf("Printing UDP segment header...\n");

        //print received header data (and server-calculated checksum)
        printf("Source Port: %hu\n", rSeg.sPort);
        printf("Dest. Port: %hu\n", rSeg.dPort);
        printf("Length: %hu\n", rSeg.len);
        printf("Checksum (server-calculated): %hu\n", rSeg.chksum);

    }
    //otherwise print failure message and send notice to client
    else
    {
        printf("Checksums do not match, discarding segment...\n");

        if (sendto(sockfd, "fail", strlen("fail"), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
    }
        
    close(sockfd);
    return 0;
}
