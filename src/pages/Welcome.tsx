import Navbar from '../components/navbar.tsx';
import '../css/welcome.css';

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
                    <img src="https://media.istockphoto.com/id/1147544807/vector/thumbnail-image-vector-graphic.jpg?s=612x612&w=0&k=20&c=rnCKVbdxqkjlcs3xH87-9gocETqpspHFXu5dIGB4wuM=" alt="Intro Image" className='intro-image' />
                </div>
            </div>
        </>
    );
}