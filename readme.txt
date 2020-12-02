Author: Jacob Everett (jae0204)
Class:  CSCE3530.002
Instructor: Dr. Robin Pottathuparambil

Make file instructions

To compile:  make
To clean:    make clean
To run: .\userver <port number>
        .\uclient <port number> <filename>
	
Usage instructions

When executed with correct parameters, uclient will attempt to connect to a server running on cse03.cse.unt.edu at the given port number. 
If successful, it will create and populate a C struct representing a UDP segment with the payload found in the ~1KB text file <filename>. 
The client will use bitwise operations to calculate a checksum for this UDP segment, and then it will send to the server. Once the server
receives the segment, it will calculate its own checksum over the data, compare the two, and, if verified, will store the payload data in an 
output file (s.out), as well as send a message to the client confirming that the data was received and successfully veriried. The client and server will then
both print the header data to their consoles before exiting, and the client will write the payload data to an outfile (c.out). 
Using diff to compare files should give the same result when original input file is compared with either output file.

Note:  This program is confirmed to work with files ranging in size from 3 bytes to 870 bytes, but should work on files up to 1KB.

Example server output:

Waiting for segment...
Checksum verified, writing payload to server.out...
Printing UDP segment header...
Source Port: 45250
Dest. Port: 7756
Length: 878
Checksum (server-calculated): 33935

Example client output:

Sending segment...
Checksum verified, writing payload to client.out...
Printing UDP segment header...
Source Port: 45250
Dest. Port: 7756
Length: 878
Checksum (client-calculated): 33935
