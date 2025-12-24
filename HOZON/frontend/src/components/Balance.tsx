import { useState, useEffect, useCallback } from 'react';
import { apiClient } from '../api/client';

interface BalanceProps {
  userId: string;
  refreshTrigger: number;
}

export function Balance({ userId, refreshTrigger }: BalanceProps) {
  const [balance, setBalance] = useState<number | null>(null);
  const [loading, setLoading] = useState(true);
  const [depositAmount, setDepositAmount] = useState('');
  const [depositing, setDepositing] = useState(false);
  const [message, setMessage] = useState<{ type: 'success' | 'error'; text: string } | null>(null);

  const fetchBalance = useCallback(async () => {
    try {
      const data = await apiClient.getBalance(userId);
      setBalance(data.balance);
    } catch (err) {
      console.error('Failed to fetch balance:', err);
    } finally {
      setLoading(false);
    }
  }, [userId]);

  useEffect(() => {
    fetchBalance();
  }, [fetchBalance, refreshTrigger]);

  const handleDeposit = async (e: React.FormEvent) => {
    e.preventDefault();
    const amount = parseFloat(depositAmount);
    if (isNaN(amount) || amount <= 0) {
      setMessage({ type: 'error', text: 'Введите корректную сумму' });
      return;
    }

    setDepositing(true);
    setMessage(null);

    try {
      const result = await apiClient.deposit(userId, amount);
      setBalance(result.balance);
      setDepositAmount('');
      setMessage({ type: 'success', text: `Счёт пополнен на ${amount.toFixed(2)} ₽` });
    } catch (err) {
      setMessage({
        type: 'error',
        text: err instanceof Error ? err.message : 'Ошибка пополнения'
      });
    } finally {
      setDepositing(false);
    }
  };

  return (
    <div className="card">
      <div className="card-header">
        <h2 className="card-title">Баланс</h2>
        <button className="btn btn-secondary" onClick={fetchBalance} disabled={loading}>
          Обновить
        </button>
      </div>

      <div className="balance-display">
        <span className="balance-label">Текущий баланс</span>
        {loading ? (
          <span className="spinner"></span>
        ) : (
          <span>{balance?.toFixed(2) ?? '0.00'} ₽</span>
        )}
      </div>

      {message && (
        <div className={`message message-${message.type}`}>{message.text}</div>
      )}

      <form onSubmit={handleDeposit} className="inline-form">
        <div className="form-group">
          <label className="form-label">Сумма пополнения</label>
          <input
            type="number"
            className="form-input"
            value={depositAmount}
            onChange={(e) => setDepositAmount(e.target.value)}
            placeholder="100.00"
            min="0.01"
            step="0.01"
            disabled={depositing}
          />
        </div>
        <button type="submit" className="btn btn-primary" disabled={depositing}>
          {depositing ? <span className="spinner"></span> : 'Пополнить'}
        </button>
      </form>
    </div>
  );
}
