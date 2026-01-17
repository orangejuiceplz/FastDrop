import { Link } from "react-router-dom";
import '../css/Navbar.css';


export default function Navbar() {
    return (
        <nav className="navbar">
            <div className="navbar-container">
                <Link to="/" className="navbar-link">Home</Link>
                <Link to="/app" className="navbar-link">Go to App</Link>
            </div>
        </nav>
    );
}