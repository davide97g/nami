import express from 'express';
import { WebSocketServer, WebSocket } from 'ws';
import { createServer } from 'http';

const app = express();
const server = createServer(app);

const PORT = process.env.PORT ? parseInt(process.env.PORT, 10) : 3000;

// REST endpoint
app.get('/', (req, res) => {
  res.send('✅ Server running');
});

// WebSocket server
const wss = new WebSocketServer({ server });

wss.on('connection', (ws: WebSocket) => {
  console.log('🔌 WebSocket client connected');

  ws.on('message', (message: Buffer) => {
    const messageStr = message.toString();
    console.log('📨 Received message:', messageStr);
    
    // Echo the message back
    ws.send(`Echo: ${messageStr}`);
  });

  ws.on('close', () => {
    console.log('🔌 WebSocket client disconnected');
  });

  ws.on('error', (error: Error) => {
    console.error('❌ WebSocket error:', error);
  });
});

server.listen(PORT, () => {
  console.log(`🚀 Server running on http://localhost:${PORT}`);
  console.log(`🔌 WebSocket server ready on ws://localhost:${PORT}`);
});

