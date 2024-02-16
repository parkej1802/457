
g++ -o server cpsc441a3server.cpp
./server

g++ -o client cpsc441a3client.cpp
./client

run server first and then run client 

server needs to enter port number for udp and tcp 
client needs to enter ip address and port number for udp and tcp 

ip address for same computer it will be 127.0.0.1 

I have test that if we type different port number with server and client, it doesn't connect. It has to have same port number for both client and server. 

if it's connect it will show 

1. Split Text (Basic Encoding)
2. Merge Text (Basic Encoding)
3. Split Text (Advanced Encoding)
4. Merge Text (Advanced Encoding)
5. Quit
Choose option:

choose option 1, it will ask for user to enter string 
enter any string then press enter then it will return vowels and consonants

for option 2, user have to put the exact return value from option 1 for consonant and vowels in order to receive same merged text of option 1 string. Need to enter correct amount of space for consonant and vowel.

option 3 works same as 4

option 4 will return no vowels for consonant and for vowel, there are numbers placed in consonant. In order to receive same value of string that we entered in option 3, user need to enter exactly same value to the consonant and vowel. Just enter same value from option 3 for vowel and consonant.