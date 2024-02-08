#! /usr/bin/env python3

# This program switches on and off the coolant for a laser engraver.
# It inspects the traffic between Lightburn and the engraver, detects GCode commands M8/M9
# and sends the aproppriate command to the connected ESP32 to switch on/off the coolant actuators.

# M4 - laser ON
# M5 - laser OFF
# M8 - coolant ON
# M9 - coolant OFF

import socket
# import telnetlib

HOST = "127.0.0.1"  # localhost
LBRN_PORT = 23
ENGRAVER_IP = "192.168.0.2" # must be adapted
ENGRAVER_PORT = 23
SWITCHER_IP = "192.168.30.27" # must be adapted
SWITCHER_PORT = 23

GCODE_LASER_ON = "M4"
GCODE_LASER_OFF = "M5"
GCODE_COOLANT_ON = "M8"
GCODE_COOLANT_OFF = "M9"

COOLANT_OFF = "OFF"
COOLANT_LOW = "LOW"
COOLANT_HIGH = "HIGH"

# def telnet_to_engraver(lbrn_client_socket: socket, data: []):
#     with telnetlib.Telnet(ENGRAVER_IP, ENGRAVER_PORT) as tn:
#         tn.write(("\n".join(data) + "\n").encode())
#         oks_in_response = 0
#         while oks_in_response < len(data):
#             response = tn.read_all()
#             oks_in_response += response.decode().count("ok")
#             lbrn_client_socket.send_all(response)

# def switch_coolant(data: str):
#     with telnetlib.Telnet(SWITCHER_IP, SWITCHER_PORT) as tn:
#         print(f"to switcher: {data}")
#         tn.write(data.encode())
#         oks_in_response = 0
#         response = tn.read_all()
#         print(f"from switcher: {response}")

def process_request(lbrn_client_socket: socket, received_data: str):
    gcode_commands = received_data.split("\n")
    commands_for_engraver = []
    for command in gcode_commands:
        if command == GCODE_LASER_ON:
            commands_for_engraver.append(command)
            send_to_engraver(lbrn_client_socket, commands_for_engraver)
            send_to_switcher(COOLANT_LOW)
        elif command == GCODE_LASER_OFF:
            commands_for_engraver.append(command)
            send_to_engraver(lbrn_client_socket, commands_for_engraver)
            send_to_switcher(COOLANT_OFF)
        elif command == GCODE_COOLANT_ON:
            send_to_engraver(lbrn_client_socket, commands_for_engraver)
            send_to_switcher(COOLANT_HIGH)
        elif command == GCODE_COOLANT_OFF:
            send_to_engraver(lbrn_client_socket, commands_for_engraver)
            send_to_switcher(COOLANT_LOW)
        else:
            commands_for_engraver.append(command)
    send_to_engraver(lbrn_client_socket, commands_for_engraver)

def send_to_engraver(lbrn_client_socket: socket = None, data: [] = None, timeout: float = 1.0) -> bool:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        sock.connect((ENGRAVER_IP, ENGRAVER_PORT))
        if data != None and lbrn_client_socket != None:
            sock.sendall(("\n".join(data) + "\n").encode())
            oks_in_response = 0
            while oks_in_response < len(data):
                response = sock.read_all()
                oks_in_response += response.decode().count("ok")
                lbrn_client_socket.send_all(response)
    except TimeoutError:
        print("No connection to engraver")
        sock.close()
        return False
    sock.close()
    return True
    
def send_to_switcher(data: str = "", timeout: float = 1.0) -> bool:
    print(f"Set coolant to {data}")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        sock.connect((SWITCHER_IP, SWITCHER_PORT))
        if data != "":
            sock.sendall(data.encode())
    except TimeoutError:
        print("No connection to switcher")
        sock.close()
        return False
    sock.close()
    return True

def start_server(localhost: str, port: int) -> socket:
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((localhost, port))
    server_socket.listen(5)
    return server_socket

def listen_lbrn(server_socket: socket):
    while True:
        client_socket, _ = server_socket.accept()
        print("Lightburn connected!")
        while True:
            received_data = client_socket.recv(1024).decode()
            if received_data == "":
                print("Lightburn disconnected!")
                send_to_switcher(COOLANT_OFF)
                break
            process_request(client_socket, received_data)

def main():
    print("Connecting to engraver...")
    if not send_to_engraver(timeout=3600):
        quit()
    print("Engraver connected!")
    print("Connecting to switcher...")
    if not send_to_switcher(timeout=3600):
        quit()
    print("Switcher connected!")
    send_to_switcher(COOLANT_LOW)
    lbrn_server_socket = start_server(HOST, LBRN_PORT)
    listen_lbrn(lbrn_server_socket)


if __name__ == "__main__":
    main()
