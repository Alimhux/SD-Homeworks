import { useState, useEffect, useCallback, useRef } from 'react';
import { apiClient, Order } from '../api/client';

interface OrderListProps {
  userId: string;
  onOrderStatusChange: () => void;
}

export function OrderList({ userId, onOrderStatusChange }: OrderListProps) {
  const [orders, setOrders] = useState<Order[]>([]);
  const [loading, setLoading] = useState(true);
  const pollingRef = useRef<number | null>(null);

  const fetchOrders = useCallback(async () => {
    try {
      const data = await apiClient.getOrders(userId);
      setOrders(data);

      const hasNewOrders = data.some(o => o.status === 'NEW');
      if (!hasNewOrders && pollingRef.current) {
        clearInterval(pollingRef.current);
        pollingRef.current = null;
      }

      return data;
    } catch (err) {
      console.error('Failed to fetch orders:', err);
      return [];
    } finally {
      setLoading(false);
    }
  }, [userId]);

  useEffect(() => {
    fetchOrders();

    return () => {
      if (pollingRef.current) {
        clearInterval(pollingRef.current);
      }
    };
  }, [fetchOrders]);

  const startPolling = useCallback(() => {
    if (pollingRef.current) return;

    pollingRef.current = window.setInterval(async () => {
      const updatedOrders = await fetchOrders();
      const hasNewOrders = updatedOrders.some((o: Order) => o.status === 'NEW');

      if (!hasNewOrders) {
        if (pollingRef.current) {
          clearInterval(pollingRef.current);
          pollingRef.current = null;
        }
        onOrderStatusChange();
      }
    }, 1000);
  }, [fetchOrders, onOrderStatusChange]);

  const getStatusBadgeClass = (status: string) => {
    switch (status) {
      case 'NEW': return 'status-badge status-new';
      case 'FINISHED': return 'status-badge status-finished';
      case 'CANCELLED': return 'status-badge status-cancelled';
      default: return 'status-badge';
    }
  };

  const getStatusLabel = (status: string) => {
    switch (status) {
      case 'NEW': return 'Обработка';
      case 'FINISHED': return 'Оплачен';
      case 'CANCELLED': return 'Отменён';
      default: return status;
    }
  };

  const formatDate = (dateStr: string) => {
    return new Date(dateStr).toLocaleString('ru-RU', {
      day: '2-digit',
      month: '2-digit',
      year: 'numeric',
      hour: '2-digit',
      minute: '2-digit'
    });
  };

  useEffect(() => {
    const hasNewOrders = orders.some(o => o.status === 'NEW');
    if (hasNewOrders) {
      startPolling();
    }
  }, [orders, startPolling]);

  return (
    <div className="card">
      <div className="card-header">
        <h2 className="card-title">Мои заказы</h2>
        <button className="btn btn-secondary" onClick={fetchOrders} disabled={loading}>
          {loading ? <span className="spinner"></span> : 'Обновить'}
        </button>
      </div>

      {loading ? (
        <div style={{ textAlign: 'center', padding: '40px' }}>
          <span className="spinner"></span>
        </div>
      ) : orders.length === 0 ? (
        <div className="empty-state">
          <div className="empty-state-icon">📦</div>
          <p>У вас пока нет заказов</p>
        </div>
      ) : (
        <div>
          {orders.map((order) => (
            <div key={order.id} className="order-item">
              <div className="order-info">
                <div className="order-description">
                  {order.description || 'Без описания'}
                </div>
                <div className="order-meta">
                  {formatDate(order.created_at)} • ID: {order.id.substring(0, 8)}...
                </div>
              </div>
              <div className="order-amount">{order.amount.toFixed(2)} ₽</div>
              <span className={getStatusBadgeClass(order.status)}>
                {order.status === 'NEW' && <span className="spinner" style={{ marginRight: '5px', width: '12px', height: '12px' }}></span>}
                {getStatusLabel(order.status)}
              </span>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
