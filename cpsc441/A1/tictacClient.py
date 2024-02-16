import socket
import sys
import os

#function for sending message to server
def send(socket, message):
    #it sends message through socket and changes the message to utf-8 form
    try:
        socket.sendall(message.encode('utf-8'))
    #catch any error whenever it has errors
    except socket.timeout:
        print(err)
        exit(1)

    except socket.error as err:
        print(err)
        exit(1)

#function for receiving message
def receive(socket):
    try:
        response = socket.recv(1024)
        return response.decode('utf-8')
    
    #catch error
    except socket.timeout:
        print("Server is Down")
        exit(1)

    except socket.error as err:
        print(err)
        exit(1)

#initialize 2d board
def board(list):
    initBoard = list.split(',')
    #split by , so i can write 2d board
    for i in range(3):
        for j in range(3):
            print(initBoard[3*i + j], end=" ")
        print()

#function to save game
def save_game(board_state):
    #make player to enter filename and if filename isn't ending with txt file then it automatically add txt at the end of file
    filename = input("Enter a filename to save the game: ")
    if not filename.endswith(".txt"):
        filename += ".txt"
    f = open(filename, "w")
    f.write(board_state)

#function to load game
def load_game():
    #player enters filename in the path, if it exist it brings saved board of file else go back to main menu
    filename = input("Enter filename to load: ")
    if not filename.endswith(".txt"):
        filename += ".txt"

    if os.path.exists(filename):
        f = open(filename, "r")
        savedboard = f.read()
        return savedboard
    else:
        print("File does not exist")
        return None

#function to play game 
def play_game(s, score, board_state):
    #save board state so it can be saved right after game started
    current_board = board_state

    while True:
        while True:
            #Player enters row and column
            move = input("\nEnter your move [row column]: ").strip()

            #check lowercase of end and save as well
            if move.lower() in ['end', 'save']:
                break

            #make sure to have single space between numbers
            rc = move.split(" ")

            #check whether it has two input
            if len(rc) != 2:
                print("Please enter two numbers separated by single space")
                continue

            row_value, col_value = rc

            #check if both value is number
            if not (row_value.isdigit() and col_value.isdigit()):
                print("Please enter valid numbers for row and column")
                continue

            #convert the values to integers
            row_num = int(row_value)
            col_num = int(col_value)

            #check if both numbers are between 0 and 2
            if 0 <= row_num <= 2 and 0 <= col_num <= 2:
                break
            else:
                print("Both numbers must be between 0-2")

        #if player type end then sends ENDG to server and receive OVER message
        #once received over message then prints out no winner 
        if move.lower() == "end":
            send(s, "ENDG")
            receiveMessage = receive(s)

            if receiveMessage.startswith("OVER"):
                print("No Winner")
                print(f"Result: {receiveMessage[5:]}")
                break
        
        #if type save then save the current board and break the loop
        elif move.lower() == "save":
            if current_board:
                save_game(current_board)
                print("Game saved successfully!")
                break

        #we now send the row and column and receive the message that it plays tictactoe with server
        else:
            send(s, f"MOVE:{move}")
            receiveMessage = receive(s)

            #if we receive BORD then we have received board state so we print out the board 
            if receiveMessage.startswith("BORD"):
                current_board = receiveMessage[5:]
                board(current_board)

            #when something went wrong then we receive error 
            elif receiveMessage.startswith("EROR"):
                print(f"Error: {receiveMessage[5:]}")
                if "NO GAME" in receiveMessage:
                    print("No game found")
                    continue
            
            #the alphabet between 5-6 will be either C or S
            elif receiveMessage.startswith("OVER"):
                winner = receiveMessage[5:6]
                if winner == "C":
                    score["human"] += 1
                elif winner == "S":
                    score["ai"] += 1
                print(f"Game Over: {receiveMessage[5:]}")
                break

    return score

#function to print score
def display_score(score):
    print(f"Human: {score['human']} vs AI: {score['ai']}")

#main function that requires the input of ip address and port 
def main(HOST, PORT):
    #connecting to server
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, int(PORT)))
        s.settimeout(3)
        #print("Socket Created")

    #catch error when connecting to server
    except socket.error as err: 
        print(err)
        exit(1)

    #initialize score
    score = {'human': 0, 'ai': 0}

    #Main menu
    while True:
        print("\nWelcome to TicTaeToe Game\n")
        print("1. New Game")
        print("2. Load Saved Game")
        print("3. Show Score")
        print("4. Exit")

        #choose number from menu to play
        number = input("\nChoose number: ")
        
        #when number is 1 is choosen it sends message "NEWG" and receive message from server
        #if messsage from server is BORD then it saves the current board and initialize the board
        if number == "1":
            send(s, "NEWG")
            receiveMessage = receive(s)
            if receiveMessage.startswith("BORD"):
                current_board = receiveMessage[5:]
                board(current_board)
                print("Type 'Save' to save the current file")
                print("Type 'End' to exit the game")
                score = play_game(s, score, current_board)

        #number 2 is to load game from saved file
        elif number == "2":
            loaded_data = load_game()
            current_board = loaded_data
            if loaded_data:
                board_state = loaded_data.split(',')
                
                #count 1 and 0 in game so I can know who to start first when gave is saved and loaded again
                count_one = board_state.count('1')
                count_zero = board_state.count('0')

                #we now know whether client is X and O
                if count_one == count_zero:
                    client_side = "X"
                else:
                    client_side = "O"

                #send message to server and receive
                send(s, f"LOAD:{client_side},{loaded_data}")
                receiveMessage = receive(s)

                #if message is BORD then it starts game from the saved file otherwise show error
                if receiveMessage.startswith("BORD"):
                    current_board = receiveMessage[5:]
                    board(receiveMessage[5:])
                    score = play_game(s, score, current_board)

                elif receiveMessage.startswith("EROR"):
                    print(f"Error: {receiveMessage[5:]}")
                    if "NO GAME" in receiveMessage:
                        print("No game found")
                        continue

        #it displays score
        elif number == "3":
            display_score(score)

        #it closes game 
        elif number == "4":
            send(s, "CLOS")
            print("See you next time!")
            break

        #if numberes aren't from the menu then let player to type again        
        else:
            print("\nChoose the number from the list")
            continue

    s.close()

#if the number of input isn't 3 then it prints error
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Type correct IP and Port")
        sys.exit(1)
    else:
        main(sys.argv[1], sys.argv[2])