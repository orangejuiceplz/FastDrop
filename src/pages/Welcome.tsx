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
            <div className='divider'></div>
            <div className='middle'>
                <h1 style={{ textAlign: 'center' }}>What is FastDrop?</h1>
                <div className='introduction-cards'>
                    <div className='introduction-card'>
                        <h2>Functions</h2>
                        <ul>
                            <li>Share files between devices quickly and securely</li>
                            <li>Supports multiple file formats including images, documents, and videos</li>
                            <li>User-friendly interface for easy navigation</li>
                            <li>Works between computers and mobile devices</li>
                        </ul>
                    </div>
                    <div className='introduction-card'>
                        <h2>Why did we make FastDrop?</h2>
                        <p>
                            We wanted to make an option to make file sharing easy and efficient.
                            Currently, file sharing between devices can be a hassle, involving multiple steps
                            and often relying on third-party services that may compromise privacy, especially
                            when transferring between a computer and a mobile device. With that in mind, we
                            created FastDrop to help users acomplish just that.
                        </p>
                    </div>
                    <div className='introduction-card'>
                        <h2>Why you should choose FastDrop</h2>
                        <p>
                            FastDrop offers a seamless and secure way to transfer files between devices.
                            Unlike other services, FastDrop doesn't require any accounts or third-party
                            integrations, ensuring your data remains private and your experience is
                            straightforward and efficient.
                        </p>
                    </div>
                </div> 
            </div>
            <div className='footer'>
                <p>©2026 NytherLabs All rights reserved</p>
            </div>
        </>
    );
}