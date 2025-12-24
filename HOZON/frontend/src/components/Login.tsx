import { useState } from 'react';
import { apiClient } from '../api/client';

interface LoginProps {
  onLogin: (userId: string) => void;
}

export function Login({ onLogin }: LoginProps) {
  const [userId, setUserId] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [isNewUser, setIsNewUser] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!userId.trim()) {
      setError('Введите ID пользователя');
      return;
    }

    setLoading(true);
    setError(null);

    try {
      if (isNewUser) {
        await apiClient.createAccount(userId.trim());
      } else {
        await apiClient.getBalance(userId.trim());
      }
      onLogin(userId.trim());
    } catch (err) {
      if (isNewUser) {
        setError(err instanceof Error ? err.message : 'Не удалось создать аккаунт');
      } else {
        setError('Пользователь не найден. Создайте новый аккаунт.');
        setIsNewUser(true);
      }
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="container" style={{ maxWidth: '500px', marginTop: '100px' }}>
      <div className="card">
        <div className="card-header">
          <h2 className="card-title">
            {isNewUser ? 'Создание аккаунта' : 'Вход в HOZON'}
          </h2>
        </div>

        {error && <div className="message message-error">{error}</div>}

        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label className="form-label">ID пользователя</label>
            <input
              type="text"
              className="form-input"
              value={userId}
              onChange={(e) => setUserId(e.target.value)}
              placeholder="Например: ivan"
              disabled={loading}
            />
          </div>

          <button
            type="submit"
            className="btn btn-primary"
            disabled={loading}
            style={{ width: '100%', justifyContent: 'center' }}
          >
            {loading ? (
              <span className="spinner"></span>
            ) : isNewUser ? (
              'Создать аккаунт'
            ) : (
              'Войти'
            )}
          </button>
        </form>

        <div style={{ textAlign: 'center', marginTop: '20px' }}>
          <button
            type="button"
            className="btn btn-secondary"
            onClick={() => {
              setIsNewUser(!isNewUser);
              setError(null);
            }}
            style={{ fontSize: '0.9em' }}
          >
            {isNewUser ? 'Уже есть аккаунт? Войти' : 'Нет аккаунта? Создать'}
          </button>
        </div>
      </div>
    </div>
  );
}
