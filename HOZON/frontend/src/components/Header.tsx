interface HeaderProps {
  userId: string | null;
  onLogout: () => void;
}

export function Header({ userId, onLogout }: HeaderProps) {
  return (
    <header className="header">
      <div className="logo">
        HO<span>Z</span>ON
      </div>
      <div className="login-section">
        {userId && (
          <div className="user-info">
            <div className="user-avatar">{userId.charAt(0).toUpperCase()}</div>
            <span>{userId}</span>
          </div>
        )}
        <nav className="nav-links">
          <a
            href="http://localhost:8888"
            target="_blank"
            rel="noopener noreferrer"
            className="nav-link"
          >
            API Docs
          </a>
          {userId && (
            <button className="nav-link" onClick={onLogout} style={{ border: 'none', cursor: 'pointer', background: 'transparent' }}>
              Выйти
            </button>
          )}
        </nav>
      </div>
    </header>
  );
}
