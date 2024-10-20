Compiling and Running the Server

g++ -o server cpsc441a3server.cpp
./server

When you run the server, it will prompt you to enter the port numbers for both UDP and TCP.

Compiling and Running the Client

g++ -o client cpsc441a3client.cpp
./client

After running the client, it will ask you to enter the server’s IP address and the port numbers for both UDP and TCP. If you are running the client on the same computer as the server, use the IP address 127.0.0.1.

The port numbers for both the server and client must match; otherwise, they will not connect.

If the connection is successful, the client will display the following options:

1. Split Text (Basic Encoding)
2. Merge Text (Basic Encoding)
3. Split Text (Advanced Encoding)
4. Merge Text (Advanced Encoding)
5. Quit
Choose option:

Option 1: Split Text (Basic Encoding)
After selecting this option, the user will be asked to enter a string. The server will return the vowels and consonants.

Option 2: Merge Text (Basic Encoding)
To use this option, the user must input the exact vowel and consonant strings returned from Option 1, including the correct number of spaces, to recreate the original string.

Option 3 and 4: Split and Merge Text (Advanced Encoding)
These options function similarly to Options 1 and 2, but with a twist.

In Option 3 (Split Advanced), the server returns no vowels for the consonant output, and the vowels are replaced by numbers in the consonant output.
In Option 4 (Merge Advanced), the user needs to enter the exact values from Option 3 for both the consonant and vowel strings to recover the original string.





