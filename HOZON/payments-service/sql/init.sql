-- Payments Service Database Schema

-- Accounts table
CREATE TABLE IF NOT EXISTS accounts (
    id UUID PRIMARY KEY,
    user_id VARCHAR(255) UNIQUE NOT NULL,
    balance DECIMAL(15, 2) DEFAULT 0.00 CHECK (balance >= 0),
    version INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Transactions table (for idempotency and audit trail)
CREATE TABLE IF NOT EXISTS transactions (
    id UUID PRIMARY KEY,
    order_id UUID UNIQUE NOT NULL,  -- UNIQUE ensures exactly-once per order
    account_id UUID NOT NULL REFERENCES accounts(id),
    amount DECIMAL(15, 2) NOT NULL,
    type VARCHAR(20) NOT NULL CHECK (type IN ('DEBIT', 'CREDIT')),
    created_at TIMESTAMP DEFAULT NOW()
);

-- Transactional Inbox (for at-least-once -> exactly-once processing)
CREATE TABLE IF NOT EXISTS inbox (
    id UUID PRIMARY KEY,
    message_id UUID UNIQUE NOT NULL,  -- Deduplication key
    payload JSONB NOT NULL,
    processed BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Transactional Outbox (for reliable message publishing)
CREATE TABLE IF NOT EXISTS outbox (
    id UUID PRIMARY KEY,
    aggregate_id UUID NOT NULL,  -- order_id
    event_type VARCHAR(50) NOT NULL,
    payload JSONB NOT NULL,
    processed BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_accounts_user_id ON accounts(user_id);
CREATE INDEX IF NOT EXISTS idx_transactions_order_id ON transactions(order_id);
CREATE INDEX IF NOT EXISTS idx_transactions_account_id ON transactions(account_id);
CREATE INDEX IF NOT EXISTS idx_inbox_message_id ON inbox(message_id);
CREATE INDEX IF NOT EXISTS idx_inbox_unprocessed ON inbox(processed) WHERE processed = FALSE;
CREATE INDEX IF NOT EXISTS idx_outbox_unprocessed ON outbox(processed) WHERE processed = FALSE;
CREATE INDEX IF NOT EXISTS idx_outbox_created_at ON outbox(created_at);
