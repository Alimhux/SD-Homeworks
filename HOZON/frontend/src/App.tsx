import { useState, useCallback } from 'react';
import { Header } from './components/Header';
import { Login } from './components/Login';
import { Balance } from './components/Balance';
import { CreateOrder } from './components/CreateOrder';
import { OrderList } from './components/OrderList';

export function App() {
  const [userId, setUserId] = useState<string | null>(() => {
    return localStorage.getItem('hozon_user_id');
  });
  const [refreshTrigger, setRefreshTrigger] = useState(0);

  const handleLogin = (id: string) => {
    setUserId(id);
    localStorage.setItem('hozon_user_id', id);
  };

  const handleLogout = () => {
    setUserId(null);
    localStorage.removeItem('hozon_user_id');
  };

  const handleOrderCreated = useCallback(() => {
    setRefreshTrigger((prev) => prev + 1);
  }, []);

  const handleOrderStatusChange = useCallback(() => {
    setRefreshTrigger((prev) => prev + 1);
  }, []);

  if (!userId) {
    return (
      <>
        <Header userId={null} onLogout={handleLogout} />
        <Login onLogin={handleLogin} />
        <footer className="footer">
          <p>HOZON v1.0.0 | Microservices Architecture with RabbitMQ</p>
        </footer>
      </>
    );
  }

  return (
    <>
      <Header userId={userId} onLogout={handleLogout} />
      <div className="container">
        <div className="grid grid-2">
          <Balance userId={userId} refreshTrigger={refreshTrigger} />
          <CreateOrder userId={userId} onOrderCreated={handleOrderCreated} />
        </div>
        <OrderList userId={userId} onOrderStatusChange={handleOrderStatusChange} />
      </div>
      <footer className="footer">
        <p>HOZON v1.0.0 | Microservices Architecture with RabbitMQ</p>
        <p>Transactional Outbox/Inbox Pattern</p>
      </footer>
    </>
  );
}
