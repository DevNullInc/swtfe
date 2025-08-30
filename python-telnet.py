#!/usr/bin/env python3
import socket
import select
import sys
import time
import termios
import tty
import os

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} host port")
        sys.exit(1)
        
    host = sys.argv[1]
    try:
        port = int(sys.argv[2])
    except ValueError:
        print(f"Error: Port must be a number")
        sys.exit(1)
    
    # Connect to the server
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print(f"Connected to {host}:{port}")
    except Exception as e:
        print(f"Error connecting to {host}:{port}: {e}")
        sys.exit(1)
    
    # Set terminal to raw mode
    old_settings = termios.tcgetattr(sys.stdin.fileno())
    try:
        tty.setraw(sys.stdin.fileno())
        tty.setcbreak(sys.stdin.fileno())
        
        while True:
            # Check for data from server or keyboard
            readable, _, _ = select.select([sock, sys.stdin], [], [], 0.1)
            
            if sock in readable:
                data = sock.recv(4096)
                if not data:  # Server closed connection
                    print("\r\nServer closed connection")
                    break
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
            
            if sys.stdin in readable:
                char = os.read(sys.stdin.fileno(), 1)
                if char == b'\x04':  # Ctrl+D to exit
                    break
                sock.sendall(char)
    
    finally:
        # Restore terminal settings
        termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, old_settings)
        sock.close()
        print("\r\nDisconnected")

if __name__ == "__main__":
    main()
