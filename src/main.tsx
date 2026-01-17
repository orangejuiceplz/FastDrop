import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import { BrowserRouter, Route, Routes } from 'react-router-dom';
import Welcome from './pages/Welcome.tsx';
import App from './pages/Share.tsx';
import Connect from './pages/Connect.tsx';
import './css/index.css';

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Welcome />} />
        <Route path="/app" element={<App />} />
        <Route path="/connect" element={<Connect />} />
      </Routes>
    </BrowserRouter>
  </StrictMode>,
)