const API_BASE = '';

export interface Account {
  id: string;
  user_id: string;
  balance: number;
  version: number;
  created_at: string;
  updated_at: string;
}

export interface Order {
  id: string;
  user_id: string;
  amount: number;
  description: string;
  status: 'NEW' | 'FINISHED' | 'CANCELLED';
  created_at: string;
  updated_at: string;
}

export interface BalanceResponse {
  user_id: string;
  balance: number;
}

export interface ErrorResponse {
  error: string;
}

class ApiClient {
  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<T> {
    const response = await fetch(`${API_BASE}${endpoint}`, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options.headers,
      },
    });

    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || 'Request failed');
    }

    return data as T;
  }

  async createAccount(userId: string): Promise<Account> {
    return this.request<Account>(`/api/accounts?user_id=${encodeURIComponent(userId)}`, {
      method: 'POST',
    });
  }

  async getBalance(userId: string): Promise<BalanceResponse> {
    return this.request<BalanceResponse>(`/api/accounts/balance?user_id=${encodeURIComponent(userId)}`);
  }

  async deposit(userId: string, amount: number): Promise<Account> {
    return this.request<Account>(`/api/accounts/deposit?user_id=${encodeURIComponent(userId)}`, {
      method: 'POST',
      body: JSON.stringify({ amount }),
    });
  }

  async createOrder(userId: string, amount: number, description: string): Promise<Order> {
    return this.request<Order>(`/api/orders?user_id=${encodeURIComponent(userId)}`, {
      method: 'POST',
      body: JSON.stringify({ amount, description }),
    });
  }

  async getOrders(userId: string): Promise<Order[]> {
    return this.request<Order[]>(`/api/orders?user_id=${encodeURIComponent(userId)}`);
  }

  async getOrder(orderId: string): Promise<Order> {
    return this.request<Order>(`/api/orders/${orderId}`);
  }

  async checkHealth(): Promise<{ status: string; service: string }> {
    return this.request<{ status: string; service: string }>('/health');
  }
}

export const apiClient = new ApiClient();
