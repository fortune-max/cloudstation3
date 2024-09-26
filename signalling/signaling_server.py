import sys
import json
import asyncio
import websockets
from typing import Optional
from websockets.legacy.server import WebSocketServerProtocol

clients = {}
curr_client_id = 0
server: Optional[WebSocketServerProtocol] = None

async def handle_server(websocket: WebSocketServerProtocol):
    global server
    if server:
        await websocket.send("Server already exists")
        return
    server = websocket
    await server.send("Server created")
    print("Server created")
    # handle pending messages
    for client in clients.values():
        offer_msg = {"client_id": client["client_id"], "offer": client["offer"]}
        await server.send(json.dumps(offer_msg))
    while True:
        try:
            msg: str = await server.recv()
        except websockets.exceptions.ConnectionClosedOK:
            print("Server closed before send")
            handle_reset()
            break
        print(f"Server << {msg}")
        msg_json: dict = json.loads(msg)
        client_id, answer = msg_json["client_id"], msg_json["answer"]
        try:
            await clients[client_id]["socket"].send(json.dumps(answer))
            await clients[client_id]["socket"].close()
            clients.pop(client_id)
        except websockets.exceptions.ConnectionClosedOK:
            print("Client closed before send")
            break
    server = None
    print("Server closed")

async def handle_client(websocket: WebSocketServerProtocol):
    global curr_client_id; curr_client_id += 1
    client_id = curr_client_id
    print(f"Client {client_id} connected")

    try:
        msg: str = await websocket.recv()
    except websockets.exceptions.ConnectionClosedOK:
        print(f"Client {client_id} closed before send")
        return
    except websockets.exceptions.ConnectionClosedError:
        print(f"Client {client_id} closed with error (probably reset handler called)")
        return
    print(f"Client {client_id} << {msg}")
    clients[client_id] = {
        "client_id": client_id,
        "socket": websocket,
        "offer": msg
    }
    if server:
        offer_msg = {"client_id": client_id, "offer": msg}
        await server.send(json.dumps(offer_msg))
        await websocket.wait_closed()
        print(f"closed {offer_msg['client_id']}")
    else:
        print("waiting for server")
        await websocket.wait_closed()
        print(f"closed {offer_msg['client_id']}")


async def handle_reset():
    print("Resetting server")
    global server, clients, curr_client_id
    if clients:
        for client in clients.values():
            await client["socket"].close()
        clients = {}
    if server:
        await server.close()
        server = None
    curr_client_id = 0

async def handle_websocket(websocket: WebSocketServerProtocol, path: str):
    if path == '/server':
        print("server")
        await handle_server(websocket)
    elif path == '/client':
        print("client")
        await handle_client(websocket)
    elif path == '/reset':
        print("reset")
        await handle_reset()

async def main():
    # Usage: ./server.py [[host:]port]
    endpoint_or_port = sys.argv[1] if len(sys.argv) > 1 else "8000"
    endpoint = endpoint_or_port if ':' in endpoint_or_port else "127.0.0.1:" + endpoint_or_port
    print(f'Listening on {endpoint}')

    host, port = endpoint.rsplit(':', 1)
    server = await websockets.serve(handle_websocket, host, int(port))
    await server.wait_closed()


if __name__ == '__main__':
    asyncio.run(main())
