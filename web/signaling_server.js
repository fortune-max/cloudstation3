const WebSocket = require('ws');

const port = process.env.SIGNAL_SERVER_PORT || 8088;

const wss = new WebSocket.Server({ port: port });

wss.on('connection', (ws) => {
  console.log('Client connected', wss.clients.size);
  ws.on('message', (message) => {
    const clientIdx = Array.from(wss.clients).indexOf(ws);
    console.log(`Message from ${clientIdx + 1}:`, message.toString());
    wss.clients.forEach((client) => {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(message.toString());
      }
    });
  });
});

console.log(`Signal server started on ws://localhost:${port}`);
