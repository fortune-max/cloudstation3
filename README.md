# cloudstation3

Play PS3 games in your browser. This was my MEng capstone at Harbour.Space (2024).

I moved to Barcelona and couldn't bring my PlayStation, so I tried to build a way to play my games from any device. The emulator (RPCS3) runs on a Linux machine with an NVIDIA GPU, the frames are encoded on the GPU and streamed to the browser over WebRTC, and your controller input goes back the other way.

Demo video: https://youtu.be/Ei86aeKOFcs

Note: this is a personal project, not a service. No games, ROMs, BIOS or firmware are included or linked. Only use games and firmware you dumped yourself from hardware you own.

## How it works

```
RPCS3 (patched) ──▶ shared memory ──▶ streamer ──▶ NVENC (H.264) ──▶ WebRTC ──▶ browser <video>
browser gamepad ──▶ WebRTC DataChannel ──▶ streamer ──▶ virtual joystick (uinput) ──▶ RPCS3
browser ──▶ signalling server ──▶ streamer   (only to set up the connection)
```

- **Getting frames out of the emulator.** I patched my fork of RPCS3 ([fortune-max/rpcs3](https://github.com/fortune-max/rpcs3)) to copy every frame into a POSIX shared memory region (`/vectframe_shm`). The streamer maps the same region and reads from it. A 720p frame is about 3.7 MB, so I didn't want to push that through a socket 25 times a second.
- **Encoding.** The streamer converts each frame to YUV with swscale and encodes it with FFmpeg's `h264_nvenc`, at 1 Mbps, no B-frames and a keyframe every 10 frames, to keep latency low.
- **Streaming.** The video goes out over WebRTC using libdatachannel, so the browser handles decoding, jitter and packet loss. Lost packets get resent (NACK).
- **Controller input.** The browser reads the gamepad with the Gamepad API and sends button and axis changes over a DataChannel on the same connection. The streamer turns them into events on a virtual joystick created with libevdev, which RPCS3 sees as a normal controller.
- **Signalling.** A small Python WebSocket server passes the browser's offer to the streamer and the answer back. After that the connection is peer to peer. There's also a mode where you copy and paste the offer and answer by hand, without the server.

## Folders

- `streamer/`: the C++ streamer (shared memory reader, NVENC encoding, WebRTC, virtual joystick).
- `web/`: the Vue 3 player.
- `signalling/`: the Python signalling server, plus notes on getting a Let's Encrypt certificate.
- `prototype-joystream/`: my first prototype from Nov 2023, which only sent gamepad input to a virtual joystick over a WebSocket.

These were separate repos while I was working on them. I merged them here and kept their history.

## Running it

**Streamer** (Linux with an NVIDIA GPU). Needs CMake 3.28+, FFmpeg built with `h264_nvenc`, libevdev and nlohmann_json. CMake downloads libdatachannel and libsrtp.

```sh
cd streamer
cmake -B build && cmake --build build
sudo ./build/frame_streamer   # needs /dev/uinput
```

Start the patched RPCS3 first, because the streamer exits if the shared memory region doesn't exist yet. The signalling URL is hardcoded in `main.cpp`.

**Signalling server**

```sh
cd signalling
pip install -r requirements.txt
python signaling_server.py 0.0.0.0:8000
```

**Player**

```sh
cd web
npm install
npm run dev
```

## What I'd improve

- The streamer just reads the latest frame every few milliseconds, so it can encode the same frame twice or catch one halfway through being written. A frame counter in shared memory would fix that.
- Audio is captured by the emulator patch but not streamed yet.
- It only uses STUN, so it won't connect through stricter NATs or networks that block UDP. That needs a TURN server.
- The bitrate is fixed, and everyone watching shares one stream and one controller.
- Resolution, the shared memory name and the signalling URL are hardcoded.

## License

MIT for my code. RPCS3 is a separate GPL-2.0 project, and this repo only links to my fork.
