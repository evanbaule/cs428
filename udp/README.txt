# CS428 Project 2

Packet structure:
    - Metadata
        // CLIENT -- > (Meta) -- > SERVER
        - op code (short - 2B)
			- META OP: 1
        - file name (char[32] - 32B)
        - file size (int - 4B)
        - number of packets (int - 4B)
         _____________________________________________(1500)__________________________________________________
        | OP  |   file_name     |  file_size   | # packets  |                    Empty                        |
        |_(2)_|______(32)_______|_____(4)______|_____(4)____|___________________(1456)________________________|

        
    - Datagram
        // CLIENT -- > (Datagram) -- > SERVER contains 1494 bytes of file data
        - op code (short - 2B) 
			- DG OP : 2
        - packet number (int - 4B)
        - data (char[1494] - 1494B)
         _____________________________________________(1500)__________________________________________________
        | OP  | packet #   |                    Data                                                          |
        |_(2)_|____(4)_____|___________________(1494)_________________________________________________________|
    
    - ACK
        // SERVER -- > (ACK) -- > CLIENT to acknowledge packet w packet number ack->packet_num
        - op code (short - 2B)
			- ACK OP : 3
        - packet number(int - 4B)
         _____________________________________________(1500)__________________________________________________
        | OP  | packet #   |                    Empty                                                         |
        |_(2)_|____(4)_____|____________________(1494)________________________________________________________|
        
    - Tail
        // CLIENT -- > (Tail) -- > SERVER
        // Used to signify the end of the transmission - tells the server that the file is done writing
        - op code (short - 2B)
			- TAIL OP: 4
         _____________________________________________(1500)__________________________________________________
        | OP  |                                 Empty                                                         |
        |_(2)_|_________________________________(1498)________________________________________________________|



    
