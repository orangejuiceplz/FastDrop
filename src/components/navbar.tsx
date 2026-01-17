import { Link } from "react-router-dom";
import '../css/navbar.css';


export default function Navbar() {
    return (
        <nav className="navbar">
            <div className="navbar-container">
                <Link to="/" className="navbar-link">Home</Link>
                <Link to="/app" className="navbar-link">Share</Link>
                <Link to="/connect" className="navbar-link">Connect</Link>
            </div>
        </nav>
    );
}