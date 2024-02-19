#! /usr/bin/env python3

# This program is a gateway between a serial port and a socket. 
# It reads data from the serial port and forwards it to the socket, and vice versa.

import socket
import threading
import time
import serial
import serial.tools.list_ports
import argparse

GCODE_LASER_ON = "M4"
GCODE_LASER_OFF = "M5"
GCODE_COOLANT_ON = "M8"
GCODE_COOLANT_OFF = "M9"

def forward_from_socket_to_serial(client_socket, ser):
    while True:
        try:
            # Receive message from socket
            data = client_socket.recv(1024)
            if not data:
                break
            print(f"Received from socket: {data.decode('utf-8')}")

            # Write message to serial port
            ser.write(data)
        except Exception as e:
            print(f"Error forwarding data from socket to serial: {e}")
            break

def forward_from_serial_to_socket(client_socket, ser):
    while True:
        try:
            # Read message from serial port
            data = ser.readline()
            if not data:
                break
            print(f"Received from serial: {data.decode('utf-8')}")

            # Send message to socket
            client_socket.sendall(data)
        except Exception as e:
            print(f"Error forwarding data from serial to socket: {e}")
            break

def listen_and_forward(port, serial_port):
    # Verify if serial port is available
    while serial_port not in [port.device for port in serial.tools.list_ports.comports()]:
        print(f"Serial port {serial_port} not found. Retrying in 5 seconds...")
        time.sleep(5)

    host_ip = "0.0.0.0"

    # Create a socket object
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to a specific address and port
    server_socket.bind((host_ip, port))

    # Listen for incoming connections (max backlog is set to 1)
    server_socket.listen(1)
    print(f"Listening for connections on {host_ip}:{port}")

    # Open the serial port
    ser = serial.Serial(serial_port)
    if not ser.is_open:
        ser.open()
    print(f"Opened serial port {serial_port}")

    try:
        while True:
            # Wait for a connection
            client_socket, client_address = server_socket.accept()
            print(f"Accepted connection from {client_address}")

            # Start a thread to forward data from socket to serial
            socket_to_serial_thread = threading.Thread(target=forward_from_socket_to_serial, args=(client_socket, ser))
            socket_to_serial_thread.start()

            # Start a thread to forward data from serial to socket
            serial_to_socket_thread = threading.Thread(target=forward_from_serial_to_socket, args=(client_socket, ser))
            serial_to_socket_thread.start()

    except KeyboardInterrupt:
        print("Interrupted")
    finally:
        # Close the serial port
        ser.close()
        print("Serial port closed")
        client_socket.close()
        server_socket.close()

def main():
    # Set the port to listen on
    port = 23

    # Create the parser
    parser = argparse.ArgumentParser(description="Listen and forward data")

    # Add the arguments
    parser.add_argument('SerialPort', metavar='serialport', type=str, help='the path to the serial port')

    # Parse the arguments
    args = parser.parse_args()

    # Now you can use args.SerialPort to get the serial port path
    serial_port = args.SerialPort

    # Start listening for incoming messages from the socket and forward them to the serial port
    listen_and_forward(port, serial_port)

if __name__ == "__main__":
    main()
