import '../css/Share.css'
import Navbar from '../components/navbar.tsx';
import { useState } from 'react';

const API_URL = 'http://localhost:1337';

function Share() {
  const [mode, setMode] = useState<'select' | 'upload' | 'receive'>('select');
  const [file, setFile] = useState<File | null>(null);
  const [password, setPassword] = useState('');
  const [code, setCode] = useState('');
  const [receiveCode, setReceiveCode] = useState('');
  const [receivePassword, setReceivePassword] = useState('');
  const [qrUrl, setQrUrl] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [fileInfo, setFileInfo] = useState<{filename: string, size: string, locked: boolean} | null>(null);

  const handleUpload = async () => {
    if (!file) return;
    setLoading(true);
    setError('');

    const formData = new FormData();
    formData.append('file', file);
    if (password) {
      formData.append('password', password);
    }

    try {
      const response = await fetch(`${API_URL}/upload`, {
        method: 'POST',
        body: formData,
      });
      const data = await response.json();
      
      if (response.ok) {
        setCode(data.code);
        setQrUrl(`${API_URL}${data.qr_url}`);
      } else {
        setError(data.error || 'Upload failed');
      }
    } catch (err) {
      setError('Failed to connect to server');
    }
    setLoading(false);
  };

  const handleCheckCode = async () => {
    if (!receiveCode) return;
    setLoading(true);
    setError('');
    setFileInfo(null);

    try {
      const response = await fetch(`${API_URL}/status/${receiveCode}`);
      const data = await response.json();
      
      if (data.found) {
        setFileInfo({ filename: data.filename, size: data.size, locked: data.locked });
      } else {
        setError('File not found');
      }
    } catch (err) {
      setError('Failed to connect to server');
    }
    setLoading(false);
  };

  const handleDownload = async () => {
    setLoading(true);
    setError('');

    try {
      const options: RequestInit = fileInfo?.locked 
        ? {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ password: receivePassword })
          }
        : { method: 'GET' };

      const response = await fetch(`${API_URL}/retrieve/${receiveCode}`, options);
      
      if (response.status === 401) {
        setError('Invalid password');
        setLoading(false);
        return;
      }

      if (!response.ok) {
        setError('Download failed');
        setLoading(false);
        return;
      }

      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = fileInfo?.filename || 'download';
      a.click();
      window.URL.revokeObjectURL(url);
      
      setFileInfo(null);
      setReceiveCode('');
      setReceivePassword('');
    } catch (err) {
      setError('Failed to download file');
    }
    setLoading(false);
  };

  if (mode === 'select') {
    return (
      <>
        <Navbar />
        <div className='connect-container'>
          <h1>FastDrop</h1>
          <h2>Share files instantly</h2>
          <div style={{ display: 'flex', gap: '1rem', marginTop: '2rem' }}>
            <button className='connect-button' onClick={() => setMode('upload')}>Send a File</button>
            <button className='connect-button' onClick={() => setMode('receive')}>Receive a File</button>
          </div>
        </div>
      </>
    );
  }

  if (mode === 'upload') {
    return (
      <>
        <Navbar />
        <div className='connect-container'>
          <h1>Send a File</h1>
          
          {!code ? (
            <>
              <input 
                type="file" 
                onChange={(e) => setFile(e.target.files?.[0] || null)}
                style={{ marginBottom: '1rem' }}
              />
              <input 
                type="password" 
                placeholder="Password (optional)"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                style={{ marginBottom: '1rem', padding: '0.5rem' }}
              />
              <button 
                className='connect-button' 
                onClick={handleUpload}
                disabled={!file || loading}
              >
                {loading ? 'Uploading...' : 'Upload'}
              </button>
            </>
          ) : (
            <>
              <h2>Your code:</h2>
              <p style={{ fontSize: '2rem', fontWeight: 'bold' }}>{code}</p>
              {qrUrl && <img src={qrUrl} alt="QR Code" style={{ width: '200px', marginTop: '1rem' }} />}
              <button className='connect-button' onClick={() => { setCode(''); setFile(null); setQrUrl(''); }}>
                Upload Another
              </button>
            </>
          )}
          
          {error && <p style={{ color: 'red' }}>{error}</p>}
          <button className='connect-button' onClick={() => setMode('select')} style={{ marginTop: '1rem' }}>Back</button>
        </div>
      </>
    );
  }

  if (mode === 'receive') {
    return (
      <>
        <Navbar />
        <div className='connect-container'>
          <h1>Receive a File</h1>
          
          {!fileInfo ? (
            <>
              <input 
                type="text" 
                placeholder="Enter code (e.g. happy-cat-42)"
                value={receiveCode}
                onChange={(e) => setReceiveCode(e.target.value)}
                style={{ marginBottom: '1rem', padding: '0.5rem', width: '250px' }}
              />
              <button 
                className='connect-button' 
                onClick={handleCheckCode}
                disabled={!receiveCode || loading}
              >
                {loading ? 'Checking...' : 'Find File'}
              </button>
            </>
          ) : (
            <>
              <p><strong>File:</strong> {fileInfo.filename}</p>
              <p><strong>Size:</strong> {fileInfo.size}</p>
              {fileInfo.locked && (
                <input 
                  type="password" 
                  placeholder="Enter password"
                  value={receivePassword}
                  onChange={(e) => setReceivePassword(e.target.value)}
                  style={{ marginBottom: '1rem', padding: '0.5rem' }}
                />
              )}
              <button 
                className='connect-button' 
                onClick={handleDownload}
                disabled={loading || (fileInfo.locked && !receivePassword)}
              >
                {loading ? 'Downloading...' : 'Download'}
              </button>
            </>
          )}
          
          {error && <p style={{ color: 'red' }}>{error}</p>}
          <button className='connect-button' onClick={() => { setMode('select'); setFileInfo(null); setReceiveCode(''); }} style={{ marginTop: '1rem' }}>Back</button>
        </div>
      </>
    );
  }

  return null;
}

export default Share
