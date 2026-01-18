import '../css/Connect.css';
import Navbar from '../components/navbar.tsx';
import { useState, useEffect, useRef } from 'react';

const API_URL = 'https://prepractical-angelia-breathlessly.ngrok-free.dev';
const WS_URL = 'wss://https://prepractical-angelia-breathlessly.ngrok-free.dev';
interface SharedFile {
    code: string;
    filename: string;
    size: string;
    locked: boolean;
}

function Connect() {
    const [mode, setMode] = useState<'select' | 'host' | 'join'>('select');
    const [sessionId, setSessionId] = useState('');
    const [joinCode, setJoinCode] = useState('');
    const [files, setFiles] = useState<SharedFile[]>([]);
    const [connected, setConnected] = useState(false);
    const [file, setFile] = useState<File | null>(null);
    const [password, setPassword] = useState('');
    const [downloadPassword, setDownloadPassword] = useState('');
    const [downloadingCode, setDownloadingCode] = useState<string | null>(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState('');
    const wsRef = useRef<WebSocket | null>(null);

    const connectWebSocket = (session: string) => {
        const ws = new WebSocket(`${WS_URL}/ws/${session}`);
        
        ws.onopen = () => {
            ws.send(JSON.stringify({ action: 'join', session_id: session }));
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.type === 'session_joined') {
                    setConnected(true);
                    setFiles(data.files || []);
                } else if (data.type === 'file_added') {
                    setFiles(prev => [...prev, {
                        code: data.code,
                        filename: data.filename,
                        size: data.size,
                        locked: data.locked
                    }]);
                } else if (data.type === 'file_removed') {
                    setFiles(prev => prev.filter(f => f.code !== data.code));
                }
            } catch (err) {}
        };

        ws.onclose = () => {
            setConnected(false);
        };

        ws.onerror = () => {
            setError('Connection failed');
            setConnected(false);
        };

        wsRef.current = ws;
    };

    const createSession = async () => {
        setLoading(true);
        setError('');
        try {
            const response = await fetch(`${API_URL}/session/create`, { method: 'POST' });
            const data = await response.json();
            setSessionId(data.session_id);
            setMode('host');
            connectWebSocket(data.session_id);
        } catch (err) {
            setError('Failed to create session');
        }
        setLoading(false);
    };

    const joinSession = () => {
        if (!joinCode.trim()) return;
        setSessionId(joinCode.trim());
        setMode('join');
        connectWebSocket(joinCode.trim());
    };

    const handleUpload = async () => {
        if (!file || !sessionId) return;
        setLoading(true);
        setError('');

        const formData = new FormData();
        formData.append('file', file);
        if (password) {
            formData.append('password', password);
        }

        try {
            const response = await fetch(`${API_URL}/session/${sessionId}/upload`, {
                method: 'POST',
                body: formData,
            });
            
            if (response.ok) {
                setFile(null);
                setPassword('');
            } else {
                const data = await response.json();
                setError(data.error || 'Upload failed');
            }
        } catch (err) {
            setError('Failed to upload file');
        }
        setLoading(false);
    };

    const handleDownload = async (fileItem: SharedFile) => {
        setLoading(true);
        setError('');

        try {
            const options: RequestInit = fileItem.locked
                ? {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: downloadPassword })
                }
                : { method: 'GET' };

            const response = await fetch(`${API_URL}/retrieve/${fileItem.code}`, options);
            
            if (response.status === 401) {
                setDownloadingCode(fileItem.code);
                setError('Password required');
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
            a.download = fileItem.filename;
            a.click();
            window.URL.revokeObjectURL(url);
            
            setFiles(prev => prev.filter(f => f.code !== fileItem.code));
            setDownloadingCode(null);
            setDownloadPassword('');
        } catch (err) {
            setError('Failed to download');
        }
        setLoading(false);
    };

    useEffect(() => {
        return () => {
            if (wsRef.current) {
                wsRef.current.close();
            }
        };
    }, []);

    if (mode === 'select') {
        return (
            <>
                <Navbar />
                <div className='connect-page'>
                    <h1>Connect Devices</h1>
                    <h2>Share files instantly between devices on the same network</h2>
                    <div className='mode-buttons'>
                        <button onClick={createSession} disabled={loading}>
                            {loading ? 'Creating...' : 'Start New Session'}
                        </button>
                        <button onClick={() => setMode('join')}>Join Session</button>
                    </div>
                    {error && <p className='error-message'>{error}</p>}
                </div>
            </>
        );
    }

    if (mode === 'join' && !connected) {
        return (
            <>
                <Navbar />
                <div className='connect-page'>
                    <h1>Join Session</h1>
                    <h2>Enter the session code from the host device</h2>
                    <div className='join-section'>
                        <input
                            type='text'
                            className='code-input'
                            placeholder='e.g. happy-cat-42'
                            value={joinCode}
                            onChange={(e) => setJoinCode(e.target.value)}
                        />
                        <button onClick={joinSession} disabled={!joinCode.trim()}>
                            Join
                        </button>
                    </div>
                    {error && <p className='error-message'>{error}</p>}
                    <button className='back-button' onClick={() => setMode('select')}>Back</button>
                </div>
            </>
        );
    }

    return (
        <>
            <Navbar />
            <div className='connect-page'>
                <div className='session-container'>
                    <div className='session-info'>
                        <h1>{mode === 'host' ? 'Your Session' : 'Connected'}</h1>
                        <div className='session-code'>{sessionId}</div>
                        <div className='connection-status'>
                            <span className={`status-dot ${connected ? '' : 'disconnected'}`}></span>
                            <span>{connected ? 'Connected' : 'Disconnected'}</span>
                        </div>
                        {mode === 'host' && (
                            <div className='qr-container'>
                                <img src={`${API_URL}/qr/${sessionId}`} alt='Session QR' />
                            </div>
                        )}
                    </div>

                    <div className='upload-section'>
                        <label className='file-input-label'>
                            <span className='upload-icon'>↑</span>
                            <span>{file ? file.name : 'Choose a file to share'}</span>
                            <input
                                type='file'
                                onChange={(e) => setFile(e.target.files?.[0] || null)}
                            />
                        </label>
                        {file && (
                            <>
                                <input
                                    type='password'
                                    className='password-input'
                                    placeholder='Password (optional)'
                                    value={password}
                                    onChange={(e) => setPassword(e.target.value)}
                                />
                                <button onClick={handleUpload} disabled={loading}>
                                    {loading ? 'Uploading...' : 'Share File'}
                                </button>
                            </>
                        )}
                    </div>

                    <div className='files-list'>
                        <h3>Shared Files</h3>
                        {files.length === 0 ? (
                            <div className='empty-state'>No files shared yet</div>
                        ) : (
                            files.map((f) => (
                                <div key={f.code} className='file-item'>
                                    <div className='file-info'>
                                        <span className='file-name'>{f.filename}</span>
                                        <span className='file-size'>{f.size}</span>
                                    </div>
                                    <div className='file-actions'>
                                        {f.locked && <span className='lock-icon'>🔒</span>}
                                        {downloadingCode === f.code && f.locked ? (
                                            <>
                                                <input
                                                    type='password'
                                                    className='password-input'
                                                    placeholder='Password'
                                                    value={downloadPassword}
                                                    onChange={(e) => setDownloadPassword(e.target.value)}
                                                    style={{ width: '120px' }}
                                                />
                                                <button
                                                    className='download-btn'
                                                    onClick={() => handleDownload(f)}
                                                    disabled={loading}
                                                >
                                                    OK
                                                </button>
                                            </>
                                        ) : (
                                            <button
                                                className='download-btn'
                                                onClick={() => handleDownload(f)}
                                                disabled={loading}
                                            >
                                                Download
                                            </button>
                                        )}
                                    </div>
                                </div>
                            ))
                        )}
                    </div>

                    {error && <p className='error-message'>{error}</p>}
                    <button className='back-button' onClick={() => {
                        if (wsRef.current) wsRef.current.close();
                        setMode('select');
                        setSessionId('');
                        setFiles([]);
                        setConnected(false);
                    }}>
                        Leave Session
                    </button>
                </div>
            </div>
        </>
    );
}

export default Connect;
