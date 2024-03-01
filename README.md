## Tic Tac Toe Game Protocol

### Commands:

1. **JOIN \<room_id\>** (2 bytes, room_id)
    - Joins the specified room to play Tic Tac Toe.

2. **START** (6 bytes)
    - Initiates the game once two players are in the room. The game starts with 'X' making the first move.

3. **MOVE \<row\> \<column\>** (9 bytes)
    - Makes a move on the Tic Tac Toe board. The coordinates (row, column) specify the position where the player wants to place their symbol. For example, "MOVE 1 2" places the symbol in the second column of the first row.

4. **QUIT**(5 bytes)
    - Disconnects from the game.

### Responses:

1. **WAITING \<player_symbol\>**
    - Informs the player that they are waiting for another player to join the room. Also, provides the symbol assigned to the player.

2. **GAME_START \<player_symbol\>**
    - Notifies the players that the game has started. Specifies which player ('X' or 'O') is making the first move.

3. **INVALID_MOVE**
    - Indicates that the player's move was invalid (e.g., trying to make a move in an already occupied position).

4. **BOARD_UPDATE \<board_state\>**
    - Sends the current state of the Tic Tac Toe board to the player. The board_state is a 3x3 matrix representing the positions of 'X', 'O', and empty spaces.

5. **TURN \<player_symbol\>**
    - Informs the player that it's their turn to make a move.

6. **WINNER \<player_symbol\>**
    - Declares the winner of the game.

7. **DRAW**
    - Announces that the game ended in a draw.

8. **OPPONENT_DISCONNECTED**
    - Notifies the player that their opponent has disconnected.

### Example Usage:

1. **Client 1:** `JOIN room_1`
   - *Server Response:* `WAITING X`

2. **Client 2:** `JOIN room_1`
   - *Server Response:* `GAME_START O`

3. **Client 1:** `START`
   - *Server Response:* `TURN X`

4. **Client 1:** `MOVE 1 1`
   - *Server Response:* `TURN O`

5. **Client 2:** `MOVE 0 0`
   - *Server Response:* `TURN X`

6. **Client 1:** `UPDATE_BOARD`
   - *Server Response:* `BOARD_UPDATE [['X', ' ', ' '], [' ', 'O', ' '], [' ', ' ', ' ']]`

7. **Client 1:** `MOVE 2 2`
   - *Server Response:* `WINNER X`

8. **Client 2:** `QUIT`
   - *Server Response:* `OPPONENT_DISCONNECTED`

### Program Classes and Structures

### Classes:

1. **Message:**
   - **Description:** A simple structure representing a message sent between clients. It includes the content of the message, the sender's socket, and the room ID.
   - **Attributes:**
     - `std::string content`: Content of the message.
     - `SOCKET senderSocket`: Sender's socket.
     - `std::string roomId`: Room ID.

2. **ChatServer:**
   - **Description:** The main class representing the Tic Tac Toe game server.
   - **Attributes:**
     - `std::map<std::string, std::vector<SOCKET>> rooms`: Maps room IDs to a vector of client sockets in each room.
     - `std::map<std::string, std::vector<std::string>> gameBoard`: Maps room IDs to the Tic Tac Toe game board.
     - `std::map<std::string, int> PlayersReady`: Maps room IDs to the number of players ready to start.
     - `std::map<SOCKET, std::string> playerSymbols`: Maps client sockets to their assigned symbols ('X' or 'O').
     - `std::map<std::string, int> currentPlayerIndexMap`: Maps room IDs to the index of the current player in the room.
     - Various mutexes and condition variables for thread safety.
   - **Methods:**
     - `addMessageToQueue()`: Adds a message to the server's message queue.
     - `broadcastMessages()`: Listens for messages in the queue and broadcasts them to the appropriate clients.
     - `broadcastMessageToAll()`: Broadcasts a message to all clients in a specific room, including the sender.
     - `broadcastUserFigures()`: Sends a message to each client in a room, indicating the symbol they are playing as.
     - `printBoard()`: Outputs the current state of the Tic Tac Toe board for a specific room.
     - `checkWinCondition()`: Checks if a player has won the game in a specific room.
     - `checkDrawCondition()`: Checks if the game in a specific room is a draw.
     - `sendBoardUpdate()`: Sends the current state of the Tic Tac Toe board to a specific client.
     - `handleClient()`: Handles communication with a client, including joining rooms, making moves, and managing game state.
     - `start()`: Initializes and starts the server, accepting client connections and handling them in separate threads.
