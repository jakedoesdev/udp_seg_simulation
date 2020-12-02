/*
    Simple udp client
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define SERVER "129.120.151.96"
#define BUFLEN 1024  //Max length of buffer

//print error and exit
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char **argv)
{
    struct sockaddr_in si_other;               //socket struct
    int sockfd, i=0, slen=sizeof(si_other);    //socket vars
    int cSum=0, tmp=0, carry=0;                //for checksum calculation
    unsigned short int cSumShort = 0;          //for calculated checksum
    char buf[BUFLEN];                          //send/recv msg buffer
    FILE *fp;                                  //input file pointer
    
    //UDP segment struct
    struct uSegment
    {
        unsigned short int sPort;
        unsigned short int dPort;
        unsigned short int len;
        unsigned short int chksum;
        char msgData[BUFLEN];
    } uSeg;

    //memset struct to 0
    memset(&uSeg, 0, sizeof(uSeg));

    //iniitialize chksum field to 0 to prevent error during chksum calculation
    //uSeg.chksum = 0;

    //check for correct usage
    if (argc != 3)
	{
		die("Usage: ./uclient <portnumber> <filename>\n");
	}

    //store destination port in segment struct
    uSeg.dPort = atoi(argv[1]);

    //open text file containing payload
    fp = fopen(argv[2], "r");

    //bzero buf and msgData in prep to store payload file
    bzero(buf, sizeof(buf));
    bzero(uSeg.msgData, sizeof(uSeg.msgData));


    //get text from file, store in buf, close file.
    char ch;
    while ((ch = getc(fp)) != EOF)
    {
        buf[i] = ch;
        i++;
    }
    fclose(fp);

    //copy payload data from buf to msgData member
    memcpy(uSeg.msgData, buf, strlen(buf));
    
    //create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    //set parameters for socket
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = atoi(argv[1]);
    
    //convert ip to binary, store in struct
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    //send empty message to server to get source port
    if (sendto(sockfd, "", strlen(""), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto()");
    }

    //recv response from server (source port stored in buf)
    bzero(buf, sizeof(buf));
    if ((recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
    {
        die("recvfrom()");
    }
    
    //convert and store the source port sent back from the server
    uSeg.sPort = (unsigned short)strtol(buf, NULL, 10);

    //populate segment length (8 bytes for header + meaningful msgData size)
    uSeg.len = 8+(strlen(uSeg.msgData));

    //create unsigned short int array with n elements.
    //ex: if uSeg.len is 521 bytes, n = (521 + 1)/2 = 261 elements in array
    int n = ((uSeg.len * 8)/16) + 1;  //same as (uSeg.len + 1)/2;
    unsigned short int chkArr[n];

    //memcpy contents of struct into array (16 bits in each element)
    memcpy(chkArr, &uSeg, uSeg.len);

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

    //populate chksum member
    uSeg.chksum = cSumShort;

    printf("Sending segment...\n");

    //try to send the UDP struct to the server
    if (sendto(sockfd, &uSeg, uSeg.len, 0, (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto()");
    }
        
    //zero buffer
    bzero(buf, sizeof(buf));

    //try to receive response from the server
    if (recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }

    //print header if server confirms checksum verification
    if (strcmp(buf, "success") == 0)
    {
        printf("Checksum verified, writing payload to client.out...\n");

        char ch;
        fp = fopen("client.out", "w");
        for (int i = 0; i<strlen(uSeg.msgData); i++)
        {
            ch = uSeg.msgData[i];
            fputc(ch, fp);
        }
        fclose(fp);

        printf("Printing UDP segment header...\n");

        printf("Source Port: %hu\n", uSeg.sPort);
        printf("Dest. Port: %hu\n", uSeg.dPort);
        printf("Length: %hu\n", uSeg.len);
        printf("Checksum (client-calculated): %hu\n", uSeg.chksum);
    }
    else
    {
        printf("No match, exiting.\n");
    }
    

    //close socket
    close(sockfd);
    return 0;
}