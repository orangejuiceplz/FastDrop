import Navbar from '../components/navbar.tsx';
import '../css/welcome.css';

export default function Welcome() {
    return (
        <>
            <Navbar />
            <h1>Welcome to FastDrop</h1>
            <h2>A fast and secure file sharing platform</h2>
        </>
    );
}