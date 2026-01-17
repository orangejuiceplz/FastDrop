import '../css/share.css'
import Navbar from '../components/navbar.tsx';

function Share() {

  return (
    <>
      <Navbar />
      <div className='connect-container'>
        <h1>Connect to another user</h1>
        <h2>Click the button below to start the connection process</h2>
        <button className='connect-button'>Connect</button>
      </div>
    </>
  )
}

export default Share
