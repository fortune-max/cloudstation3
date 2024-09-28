# cloudstation3 web player

This is the browser client for cloudstation3, built with Vue 3, Vuetify 3 and Vite. It receives the H.264 game stream over WebRTC and sends controller input back over a WebRTC DataChannel.

Live build: https://cloudstation3.vercel.app. It only shows video while a streamer host is online.

## What it does

- Creates an `RTCPeerConnection` with STUN only (`stun.l.google.com:19302`), a recvonly video transceiver and a `gamepad` DataChannel. It waits for ICE gathering to complete, then sends the whole offer.
- **Signalling:** sends the offer to the signalling server (default `wss://ws.cloudstation3.com/client`, set in `src/components/Game.vue` and editable in the UI) and applies the answer it gets back. If you turn off "Use Signaling Server", the offer is copied to the clipboard and you paste the answer in by hand.
- **Input:** polls `navigator.getGamepads()` on every `requestAnimationFrame` and diffs the result against the previous state. Each changed button or axis is converted to a Linux evdev event `{type, code, value}` using `src/components/ps5_gamepad.cfg.json` and `src/utils.ts`. Each frame's batch is sent over the DataChannel as JSON.
- Shows ICE and signalling state, recent input events, and the received FPS (taken from `getStats()` `inbound-rtp`).

`signaling_server.js` is an early Node relay that broadcasts every message to all other connected clients (port `SIGNAL_SERVER_PORT`, default 8088). The current server is the Python one in `../signalling/`.

## Develop

```sh
npm install
npm run dev       # Vite dev server
npm run build     # vue-tsc --noEmit && vite build
npm run lint
```

See the [top-level README](../README.md) for the full architecture.
