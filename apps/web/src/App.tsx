import { useState, useEffect } from 'react';

const App = () => {
  const [status, setStatus] = useState<string>('Loading...');
  const [info, setInfo] = useState<any>(null);

  useEffect(() => {
    const fetchStatus = async () => {
      try {
        const response = await fetch('http://localhost:3000/');
        const text = await response.text();
        setStatus(text);
      } catch (error) {
        setStatus('Server not available');
      }
    };

    const fetchInfo = async () => {
      try {
        const response = await fetch('http://localhost:3000/info');
        const data = await response.json();
        setInfo(data);
      } catch (error) {
        console.error('Failed to fetch info:', error);
      }
    };

    fetchStatus();
    fetchInfo();
  }, []);

  return (
    <div className="min-h-screen bg-gray-50 py-8 px-4">
      <div className="max-w-4xl mx-auto">
        <h1 className="text-3xl font-bold text-gray-900 mb-8">Nami Server Dashboard</h1>
        
        <div className="bg-white rounded-lg shadow p-6 mb-6">
          <h2 className="text-xl font-semibold text-gray-800 mb-4">Server Status</h2>
          <p className="text-gray-600 mb-2">{status}</p>
          {info?.version && (
            <p className="text-sm text-gray-500">
              <span className="font-medium">Version:</span> {info.version}
            </p>
          )}
        </div>

        {info && (
          <div className="bg-white rounded-lg shadow p-6">
            <h2 className="text-xl font-semibold text-gray-800 mb-4">System Information</h2>
            <div className="space-y-4">
              <div>
                <h3 className="font-medium text-gray-700 mb-2">System</h3>
                <div className="bg-gray-50 rounded p-3 text-sm">
                  <p><span className="font-medium">Hostname:</span> {info.system?.hostname}</p>
                  <p><span className="font-medium">Platform:</span> {info.system?.platform}</p>
                  <p><span className="font-medium">Architecture:</span> {info.system?.arch}</p>
                  <p><span className="font-medium">Uptime:</span> {Math.floor((info.system?.uptime || 0) / 60)} minutes</p>
                </div>
              </div>

              {info.cpu && (
                <div>
                  <h3 className="font-medium text-gray-700 mb-2">CPU</h3>
                  <div className="bg-gray-50 rounded p-3 text-sm">
                    <p><span className="font-medium">Model:</span> {info.cpu.model}</p>
                    <p><span className="font-medium">Cores:</span> {info.cpu.cores}</p>
                    <p><span className="font-medium">Speed:</span> {info.cpu.speed} MHz</p>
                  </div>
                </div>
              )}

              {info.memory && (
                <div>
                  <h3 className="font-medium text-gray-700 mb-2">Memory</h3>
                  <div className="bg-gray-50 rounded p-3 text-sm">
                    <p><span className="font-medium">Total:</span> {(info.memory.total / 1024 / 1024 / 1024).toFixed(2)} GB</p>
                    <p><span className="font-medium">Used:</span> {(info.memory.used / 1024 / 1024 / 1024).toFixed(2)} GB</p>
                    <p><span className="font-medium">Free:</span> {(info.memory.free / 1024 / 1024 / 1024).toFixed(2)} GB</p>
                  </div>
                </div>
              )}

              {info.raspberryPi && (
                <div>
                  <h3 className="font-medium text-gray-700 mb-2">Raspberry Pi</h3>
                  <div className="bg-gray-50 rounded p-3 text-sm">
                    {info.raspberryPi.model && <p><span className="font-medium">Model:</span> {info.raspberryPi.model}</p>}
                    {info.raspberryPi.deviceModel && <p><span className="font-medium">Device Model:</span> {info.raspberryPi.deviceModel}</p>}
                    {info.raspberryPi.temperature && (
                      <p><span className="font-medium">Temperature:</span> {info.raspberryPi.temperature.celsius.toFixed(1)}°C</p>
                    )}
                  </div>
                </div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default App;

