#!/usr/bin/env python3
import socket
import sys
import threading
import time

if len(sys.argv) != 2:
    print("Usage: python client_test.py <PORT>")
    sys.exit(1)

HOST = "127.0.0.1"
PORT = int(sys.argv[1])

# Eventos globais para sincronização
connected_event = threading.Event()
disconnected_event = threading.Event()

def run_client(client_id, messages, delay_before_connect=0, delay_after_messages=2, wait_for_client_id=None, wait_for_disconnect_id=None):
    def receive_msg(sock, file):
        while True:
            try:
                data = sock.recv(1024)
                if not data:
                    break
                message = data.decode()
                print(f"Client {client_id} received: {message}", end='')
                file.write(message)
                file.flush()

                if wait_for_client_id is not None and f"server: client {wait_for_client_id} just arrived" in message:
                    connected_event.set()

                if wait_for_disconnect_id is not None and f"server: client {wait_for_disconnect_id} just left" in message:
                    disconnected_event.set()
            except:
                break

    try:
        time.sleep(delay_before_connect)  # Delay before connecting (for client1)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock, open(f'client{client_id}_output.txt', 'w') as file:
            sock.connect((HOST, PORT))

            # Start thread to receive messages from the server
            thread_receive = threading.Thread(target=receive_msg, args=(sock, file))
            thread_receive.daemon = True
            thread_receive.start()

            if client_id == 0 and wait_for_client_id is not None:
                connected_event.wait()

            # Send test messages
            for msg in messages:
                sock.send(msg.encode())

            if client_id == 1 and wait_for_disconnect_id is not None:
                disconnected_event.wait()

            # Add these lines to disconnect explicitly
            sock.close()
    except Exception as e:
        print(f"Client {client_id} - Error connecting with server: {e}")

# Messages for client0
messages_client0 = [
    "Hello, this is a test from client0.\n",
    "Sending multiple messages.\nAnother message from client0.\n",
    "Message without newline at the end",
    "Messages with many\nnewlines\n\nhere.\n"
]

# Messages for client1
messages_client1 = [
    "Hello from client1.\n"
]

# Create threads for each client
thread_client0 = threading.Thread(target=run_client, args=(0, messages_client0), kwargs={'wait_for_client_id': 1})
thread_client1 = threading.Thread(target=run_client, args=(1, messages_client1, 1), kwargs={'wait_for_disconnect_id': 0})

# Start client0 first
thread_client0.start()

# Start client1 after client0
thread_client1.start()

# Wait for both clients to finish
thread_client0.join()
thread_client1.join()



