-- Orders Service Database Schema

-- Orders table
CREATE TABLE IF NOT EXISTS orders (
    id UUID PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    amount DECIMAL(15, 2) NOT NULL CHECK (amount > 0),
    description TEXT,
    status VARCHAR(20) DEFAULT 'NEW' CHECK (status IN ('NEW', 'FINISHED', 'CANCELLED')),
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
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
CREATE INDEX IF NOT EXISTS idx_orders_user_id ON orders(user_id);
CREATE INDEX IF NOT EXISTS idx_orders_status ON orders(status);
CREATE INDEX IF NOT EXISTS idx_orders_created_at ON orders(created_at);
CREATE INDEX IF NOT EXISTS idx_outbox_unprocessed ON outbox(processed) WHERE processed = FALSE;
CREATE INDEX IF NOT EXISTS idx_outbox_created_at ON outbox(created_at);
