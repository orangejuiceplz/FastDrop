import Navbar from '../components/navbar.tsx';
import '../css/welcome.css';
import image from '../assets/sharing-icon.png';

export default function Welcome() {
    return (
        <>
            <Navbar />
            <div className='intro'>
                <div className='intro-text'>
                    <h1>Welcome to FastDrop</h1>
                    <h2>A fast and secure file sharing platform</h2>
                </div>
                <div className='image-container'>
                    <img src={image} alt="Intro Image" className='intro-image' />
                </div>
            </div>
        </>
    );
}