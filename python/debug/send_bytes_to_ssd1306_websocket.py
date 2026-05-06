import asyncio
import websockets

HOST = "127.0.0.1"
PORT = 3617

connected_clients = set()


# Modern handler signature (websockets >= 12)
async def handler(websocket):
    connected_clients.add(websocket)
    print(f"Client connected. Total clients: {len(connected_clients)}")
    
    try:
        async for message in websocket:
            # You can handle messages from Godot here if needed
            pass
    finally:
        connected_clients.remove(websocket)
        print(f"Client disconnected. Total clients: {len(connected_clients)}")


async def broadcast(data: bytes | bytearray):
    if not connected_clients:
        return
    
    await asyncio.gather(
        *[client.send(data) for client in connected_clients],
        return_exceptions=True
    )


async def run_test_pattern():
    buffer = bytearray(1024)
    print("Starting test pattern broadcast (progressive bits)...")
    
    while True:
        for i in range(1024):
            for bit in range(8):
                buffer[i] |= (1 << bit)
                await broadcast(buffer)
                await asyncio.sleep(0.008)  # ~125 FPS feel


async def main():
    # Important: do NOT pass the old 'ping_interval' etc. if not needed
    server = await websockets.serve(handler, HOST, PORT)
    
    print(f"✅ WebSocket server running on ws://{HOST}:{PORT}")
    await run_test_pattern()


if __name__ == "__main__":
    asyncio.run(main())