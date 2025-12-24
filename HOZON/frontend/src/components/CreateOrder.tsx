import { useState } from 'react';
import { apiClient } from '../api/client';

interface CreateOrderProps {
  userId: string;
  onOrderCreated: () => void;
}

export function CreateOrder({ userId, onOrderCreated }: CreateOrderProps) {
  const [amount, setAmount] = useState('');
  const [description, setDescription] = useState('');
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState<{ type: 'success' | 'error'; text: string } | null>(null);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();

    const orderAmount = parseFloat(amount);
    if (isNaN(orderAmount) || orderAmount <= 0) {
      setMessage({ type: 'error', text: 'Введите корректную сумму' });
      return;
    }

    if (!description.trim()) {
      setMessage({ type: 'error', text: 'Введите описание заказа' });
      return;
    }

    setLoading(true);
    setMessage(null);

    try {
      await apiClient.createOrder(userId, orderAmount, description.trim());
      setAmount('');
      setDescription('');
      setMessage({ type: 'success', text: 'Заказ создан! Ожидайте подтверждения оплаты...' });
      onOrderCreated();
    } catch (err) {
      setMessage({
        type: 'error',
        text: err instanceof Error ? err.message : 'Ошибка создания заказа'
      });
    } finally {
      setLoading(false);
    }
  };

  const quickProducts = [
    { name: 'Ноутбук', price: 49999 },
    { name: 'Смартфон', price: 29999 },
    { name: 'Наушники', price: 4999 },
    { name: 'Клавиатура', price: 2999 },
  ];

  const handleQuickProduct = (product: { name: string; price: number }) => {
    setDescription(product.name);
    setAmount(product.price.toString());
  };

  return (
    <div className="card">
      <div className="card-header">
        <h2 className="card-title">Новый заказ</h2>
      </div>

      <div style={{ marginBottom: '20px' }}>
        <label className="form-label">Быстрый выбор:</label>
        <div style={{ display: 'flex', gap: '10px', flexWrap: 'wrap' }}>
          {quickProducts.map((product) => (
            <button
              key={product.name}
              type="button"
              className="btn btn-secondary"
              onClick={() => handleQuickProduct(product)}
              style={{ fontSize: '0.85em', padding: '8px 16px' }}
            >
              {product.name} — {product.price.toLocaleString('ru-RU')} ₽
            </button>
          ))}
        </div>
      </div>

      {message && (
        <div className={`message message-${message.type}`}>{message.text}</div>
      )}

      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label className="form-label">Описание товара</label>
          <input
            type="text"
            className="form-input"
            value={description}
            onChange={(e) => setDescription(e.target.value)}
            placeholder="Например: Ноутбук ASUS"
            disabled={loading}
          />
        </div>

        <div className="form-group">
          <label className="form-label">Сумма (₽)</label>
          <input
            type="number"
            className="form-input"
            value={amount}
            onChange={(e) => setAmount(e.target.value)}
            placeholder="99.99"
            min="0.01"
            step="0.01"
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
            <>
              <span className="spinner"></span>
              Создание заказа...
            </>
          ) : (
            'Создать заказ'
          )}
        </button>
      </form>
    </div>
  );
}
