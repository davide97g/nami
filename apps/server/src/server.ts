import express from 'express';
import cors from 'cors';
import { WebSocketServer, WebSocket } from 'ws';
import { createServer } from 'http';
import os from 'os';
import { readFile } from 'fs/promises';
import { existsSync } from 'fs';
import packageJson from '../package.json' assert { type: 'json' };

const app = express();
app.use(cors());
app.use(express.json());
const server = createServer(app);

const PORT = process.env.PORT ? parseInt(process.env.PORT, 10) : 3000;

// Helper function to get Raspberry Pi information
const getRaspberryPiInfo = async () => {
  const info: Record<string, any> = {
    version: packageJson.version,
    system: {
      hostname: os.hostname(),
      platform: os.platform(),
      arch: os.arch(),
      uptime: os.uptime(),
      loadAverage: os.loadavg(),
    },
    cpu: {
      model: os.cpus()[0]?.model || 'Unknown',
      cores: os.cpus().length,
      speed: os.cpus()[0]?.speed || 0,
    },
    memory: {
      total: os.totalmem(),
      free: os.freemem(),
      used: os.totalmem() - os.freemem(),
    },
    network: os.networkInterfaces(),
  };

  // Try to get Raspberry Pi specific information
  try {
    // Read CPU info for Raspberry Pi model
    if (existsSync('/proc/cpuinfo')) {
      const cpuInfo = await readFile('/proc/cpuinfo', 'utf-8');
      const modelMatch = cpuInfo.match(/Model\s*:\s*(.+)/i);
      const revisionMatch = cpuInfo.match(/Revision\s*:\s*(.+)/i);
      const serialMatch = cpuInfo.match(/Serial\s*:\s*(.+)/i);
      
      if (modelMatch) {
        info.raspberryPi = {
          ...info.raspberryPi,
          model: modelMatch[1].trim(),
        };
      }
      if (revisionMatch) {
        info.raspberryPi = {
          ...info.raspberryPi,
          revision: revisionMatch[1].trim(),
        };
      }
      if (serialMatch) {
        info.raspberryPi = {
          ...info.raspberryPi,
          serial: serialMatch[1].trim(),
        };
      }
    }

    // Try to read device tree model
    if (existsSync('/proc/device-tree/model')) {
      const model = await readFile('/proc/device-tree/model', 'utf-8');
      info.raspberryPi = {
        ...info.raspberryPi,
        deviceModel: model.trim(),
      };
    }

    // Try to read CPU temperature
    if (existsSync('/sys/class/thermal/thermal_zone0/temp')) {
      const temp = await readFile('/sys/class/thermal/thermal_zone0/temp', 'utf-8');
      const tempCelsius = parseInt(temp.trim(), 10) / 1000;
      info.raspberryPi = {
        ...info.raspberryPi,
        temperature: {
          celsius: tempCelsius,
          fahrenheit: (tempCelsius * 9) / 5 + 32,
        },
      };
    }
  } catch (error) {
    // If we can't read Raspberry Pi specific files, continue without them
    console.warn('Could not read some Raspberry Pi specific information:', error);
  }

  return info;
};

// REST endpoints
app.get('/', (req, res) => {
  res.send('✅ Server running');
});

app.get('/info', async (req, res) => {
  try {
    const info = await getRaspberryPiInfo();
    res.json(info);
  } catch (error) {
    res.status(500).json({ error: 'Failed to retrieve system information' });
  }
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

