#!/usr/bin/env python3

"""
Fallback Telnet Server for SWR MUD
THE FINAL EPISODE

Simple maintenance server when main MUD is down
"""

import socket
import threading
import time
import sys
import signal

class FallbackServer:
    def __init__(self, port=4848, max_connections=10):
        self.port = port
        self.max_connections = max_connections
        self.running = True
        self.connections = []
        
        self.message = """
\033[1;36m═══════════════════════════════════════════════════════════════════════════════\033[0m
\033[1;33m                    Star Wars Reality MUD - Server Status\033[0m
\033[1;36m═══════════════════════════════════════════════════════════════════════════════\033[0m

\033[1;31mSERVER TEMPORARILY UNAVAILABLE\033[0m

The main game server is currently down for maintenance or experiencing
technical difficulties. Our administrators have been notified and are
working to restore service.

\033[1;32mWhat you can do:\033[0m
• Check back in a few minutes
• Visit our website for updates
• Contact us for urgent issues: support@renegadeinc.net

\033[1;33mServer Status:\033[0m Maintenance Mode (Fallback Active)
\033[1;33mPort:\033[0m {port}
\033[1;33mLast Update:\033[0m {timestamp}
\033[1;33mFallback Server:\033[0m Online since {server_start}

\033[0;90mConnection will close in 30 seconds...\033[0m

"""
        
        # Store server start time
        self.server_start_time = time.strftime('%Y-%m-%d %H:%M:%S')

    def handle_client(self, client_socket, client_address):
        """Handle individual client connection"""
        try:
            timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
            print(f"\033[32m[{timestamp}] Client connected from: {client_address[0]}:{client_address[1]}\033[0m")
            
            # Send welcome message
            message = self.message.format(
                timestamp=timestamp,
                port=self.port,
                server_start=self.server_start_time
            )
            client_socket.send(message.encode('utf-8'))
            
            # Wait 30 seconds
            time.sleep(30)
            
            # Send closing message
            client_socket.send(b"\nConnection closed.\n")
            
        except Exception as e:
            print(f"\033[31mError handling client {client_address}: {e}\033[0m")
        finally:
            try:
                client_socket.close()
                timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
                print(f"\033[33m[{timestamp}] Client disconnected: {client_address[0]}:{client_address[1]}\033[0m")
            except:
                pass
            
            # Remove from connections list
            try:
                self.connections.remove(client_socket)
            except:
                pass

    def start(self):
        """Start the fallback server"""
        # Check if port is available before attempting to bind
        self._check_port_availability()
        
        # Create socket
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            server_socket.bind(('', self.port))
            server_socket.listen(self.max_connections)
            
            print("\033[36m═══════════════════════════════════════════════════════════════════════════════\033[0m")
            print("\033[33m                        SWR MUD FALLBACK SERVER\033[0m")
            print("\033[36m═══════════════════════════════════════════════════════════════════════════════\033[0m")
            print()
            print(f"\033[33mPort:\033[0m \033[32m{self.port}\033[0m")
            print(f"\033[33mMax Connections:\033[0m \033[32m{self.max_connections}\033[0m")
            print(f"\033[33mStatus:\033[0m \033[31mMain server is DOWN - Fallback active\033[0m")
            print(f"\033[33mStarted:\033[0m \033[36m{time.strftime('%Y-%m-%d %H:%M:%S')}\033[0m")
            print()
            print("\033[34mListening for connections... (Ctrl+C to stop)\033[0m")
            print()
            
            while self.running:
                try:
                    client_socket, client_address = server_socket.accept()
                    
                    # Check connection limit
                    if len(self.connections) >= self.max_connections:
                        client_socket.send(b"Server is busy. Please try again later.\n")
                        client_socket.close()
                        continue
                    
                    self.connections.append(client_socket)
                    
                    # Handle client in separate thread
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(client_socket, client_address)
                    )
                    client_thread.daemon = True
                    client_thread.start()
                    
                except socket.error as e:
                    if self.running:
                        print(f"\033[31mSocket error: {e}\033[0m")
                        break
                except KeyboardInterrupt:
                    break
                    
        except OSError as e:
            if e.errno == 98:  # Address already in use
                print(f"\033[31mError: Port {self.port} is already in use.\033[0m")
                print("This usually means:")
                print("• Another MUD server is running on this port")
                print("• A previous fallback server is still running")
                print("• The main server is actually running")
                print()
                print("Please check:")
                print(f"  ps aux | grep {self.port}")
                print(f"  ss -tlnp | grep :{self.port}")
                print()
                return 1
            else:
                print(f"\033[31mError: Cannot bind to port {self.port}: {e}\033[0m")
                return 1
        finally:
            self.stop()
            server_socket.close()
            
        return 0

    def _check_port_availability(self):
        """Check if the target port is available"""
        import subprocess
        
        try:
            # Check what's using the port
            result = subprocess.run(
                ['ss', '-tlnp', f'sport = :{self.port}'], 
                capture_output=True, 
                text=True, 
                timeout=5
            )
            
            if result.stdout.strip():
                print(f"\033[33mWarning: Port {self.port} appears to be in use:\033[0m")
                for line in result.stdout.strip().split('\n')[1:]:  # Skip header
                    print(f"  {line}")
                print()
                print("\033[33mFallback server will attempt to bind anyway (SO_REUSEADDR enabled)\033[0m")
                print()
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError, FileNotFoundError):
            # If ss command fails, just continue
            pass

    def stop(self):
        """Stop the server"""
        print()
        print("\033[33mShutting down fallback server...\033[0m")
        self.running = False
        
        # Close all client connections
        for client_socket in self.connections[:]:
            try:
                client_socket.close()
            except:
                pass
        
        print("\033[32mFallback server stopped.\033[0m")

def signal_handler(signum, frame):
    """Handle shutdown signals"""
    print("\nReceived shutdown signal...")
    sys.exit(0)

if __name__ == "__main__":
    # Set up signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Parse command line arguments
    port = 4848
    if len(sys.argv) > 1:
        if sys.argv[1] in ['--help', '-h', 'help']:
            print("SWR MUD Fallback Server")
            print("Usage: fallback_server.py [port]")
            print("  port    Port number to listen on (default: 4848)")
            print("")
            print("This server provides a maintenance message when the main MUD is down.")
            sys.exit(0)
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("Error: Port must be a number")
            print("Usage: fallback_server.py [port]")
            sys.exit(1)
    
    # Start server
    server = FallbackServer(port)
    sys.exit(server.start())
