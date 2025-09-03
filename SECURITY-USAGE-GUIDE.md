# ZeroTier One - Security Enhancements Usage Guide

## Enhanced Authentication

### Generating Secure Tokens

```bash
# Generate token with read-only scope
curl -X POST http://localhost:9993/security/token \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "client_id": "api_user_1",
    "scopes": ["api:read"]
  }'

# Response:
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "client_id": "api_user_1",
  "scopes": ["api:read"],
  "expires_in": 3600
}
```

### Using Scoped Tokens

```bash
# Read operation (allowed)
curl -X GET http://localhost:9993/status \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."

# Write operation attempt (denied)
curl -X POST http://localhost:9993/network/8056c2e21c000001 \
  -H "Authorization: Bearer READ_ONLY_TOKEN" \
  -d '{"name":"Test Network"}'
# Returns: 401 Unauthorized
```

## Multi-Factor Authentication (MFA)

### Initial Setup

```bash
# 1. Configure MFA for a client
curl -X GET http://localhost:9993/security/mfa/setup \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "X-Client-ID: admin_user"

# Response:
{
  "mfa_enabled": true,
  "setup_uri": "otpauth://totp/ZeroTier:admin_user?secret=JBSWY3DPEHPK3PXP&issuer=ZeroTier&algorithm=SHA1&digits=6&period=30",
  "message": "MFA has been initialized. Please configure your authenticator app."
}
```

### Using MFA with Operations

```bash
# 2. Check MFA status
curl -X GET http://localhost:9993/security/mfa/status \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "X-Client-ID: admin_user"

# 3. Critical operation requiring MFA
curl -X POST http://localhost:9993/controller/network/8056c2e21c000001 \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "X-ZT1-MFA-Challenge: challenge_id_from_first_attempt" \
  -H "X-ZT1-MFA-Code: 123456" \
  -d '{"name":"Critical Network"}'
```

### Complete MFA Flow

```bash
# Step 1: First attempt without MFA (generates challenge)
curl -X POST http://localhost:9993/controller/network/new \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -d '{"name":"Secure Network"}'

# Response:
# HTTP 401 Unauthorized
# X-ZT1-MFA-Challenge: abc123def456
# X-ZT1-MFA-Required: true
{
  "error": "MFA required",
  "mfa_challenge": "abc123def456"
}

# Step 2: Retry with MFA code
curl -X POST http://localhost:9993/controller/network/new \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "X-ZT1-MFA-Challenge: abc123def456" \
  -H "X-ZT1-MFA-Code: 789012" \
  -d '{"name":"Secure Network"}'
```

## Bandwidth Management

### Policy Configuration

```bash
# Set bandwidth limits for a network
curl -X POST http://localhost:9993/security/bandwidth/policy \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "network_id": "8056c2e21c000001",
    "max_bandwidth": 10485760,
    "burst_size": 20971520,
    "qos_enabled": true,
    "traffic_shaping": true
  }'

# Configure custom QoS
curl -X POST http://localhost:9993/security/bandwidth/qos \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "network_id": "8056c2e21c000001",
    "class_name": "video_streaming",
    "priority": 6,
    "max_latency": 75.0,
    "min_bandwidth": 2097152
  }'
```

### Bandwidth Monitoring

```bash
# View general statistics
curl -X GET http://localhost:9993/security/bandwidth/stats \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"

# Response:
{
  "networks": [
    {
      "networkId": "8056c2e21c000001",
      "bytesIn": 1048576000,
      "bytesOut": 2097152000,
      "packetsIn": 1000000,
      "packetsOut": 1500000,
      "droppedPackets": 150,
      "avgLatency": 45.2,
      "packetLoss": 0.015,
      "congestionLevel": 0.12
    }
  ]
}

# Specific peer statistics
curl -X GET "http://localhost:9993/security/bandwidth/stats?network=8056c2e21c000001&peer=1234567890abcdef" \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"
```

## Security Audit and Logging

### Viewing Audit Logs

```bash
# All audit logs
curl -X GET http://localhost:9993/security/audit \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"

# Logs by severity level
curl -X GET "http://localhost:9993/security/audit?level=WARNING" \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"

# Logs since specific timestamp
curl -X GET "http://localhost:9993/security/audit?since=1640995200000&limit=50" \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"
```

### Audit Event Types

```json
{
  "logs": [
    {
      "timestamp": 1640995200000,
      "event": "auth_failed",
      "level": "WARNING",
      "client_ip": "192.168.1.100",
      "message": "API authentication failed",
      "endpoint": "/network/8056c2e21c000001"
    },
    {
      "timestamp": 1640995260000,
      "event": "mfa_success",
      "level": "INFO",
      "client_ip": "192.168.1.100",
      "message": "MFA verification successful for /controller/network",
      "endpoint": "/controller/network"
    },
    {
      "timestamp": 1640995320000,
      "event": "rate_limited",
      "level": "WARNING",
      "client_ip": "10.0.0.50",
      "message": "Client rate limited due to excessive requests",
      "endpoint": "/status"
    }
  ]
}
```

## Rate Limiting

### Rate Limiting Behavior

```bash
# Normal requests (allowed)
for i in {1..50}; do
  curl -X GET http://localhost:9993/status \
    -H "Authorization: Bearer YOUR_TOKEN"
  sleep 1
done

# Excessive requests (limited)
for i in {1..200}; do
  curl -X GET http://localhost:9993/status \
    -H "Authorization: Bearer YOUR_TOKEN" &
done

# After exceeding limit:
# HTTP 429 Too Many Requests
{
  "error": "Rate limit exceeded",
  "retry_after": 60,
  "current_limit": 50
}
```

### Checking Rate Limits

```bash
# Response headers include rate limiting information
curl -I http://localhost:9993/status \
  -H "Authorization: Bearer YOUR_TOKEN"

# Response headers:
# X-RateLimit-Limit: 100
# X-RateLimit-Remaining: 85
# X-RateLimit-Reset: 1640995800
```

## Security Configuration

### Configuration via Environment Variables

```bash
# Configure rate limiting defaults
export ZT_SECURITY_RATE_LIMIT_DEFAULT=200
export ZT_SECURITY_RATE_LIMIT_BURST=50

# Configure MFA parameters
export ZT_MFA_TIME_STEP=30
export ZT_MFA_CHALLENGE_EXPIRY=300

# Configure audit logging
export ZT_AUDIT_LOG_LEVEL=INFO
export ZT_AUDIT_LOG_RETENTION_DAYS=90

# Configure bandwidth defaults
export ZT_BANDWIDTH_DEFAULT_LIMIT=1048576
export ZT_BANDWIDTH_QOS_ENABLED=true
```

### Configuration via API

```bash
# Configure global security policies
curl -X POST http://localhost:9993/security/config \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "rate_limiting": {
      "default_limit": 100,
      "burst_multiplier": 2,
      "blacklist_threshold": 0.9
    },
    "mfa": {
      "required_endpoints": [
        "/controller/*",
        "/network/*/delete",
        "/config/settings"
      ],
      "challenge_expiry_minutes": 5
    },
    "audit": {
      "log_all_requests": false,
      "log_security_events": true,
      "retention_days": 30
    }
  }'
```

## Emergency Scenarios

### Emergency MFA Disable

```bash
# For emergency cases (root admin only)
curl -X POST http://localhost:9993/security/emergency/disable-mfa \
  -H "Authorization: Bearer ROOT_ADMIN_TOKEN" \
  -H "X-Emergency-Reason: Lost MFA device" \
  -d '{"client_id":"locked_admin","emergency_code":"EMERGENCY_OVERRIDE_CODE"}'
```

### Rate Limiting Reset

```bash
# Remove IP from blacklist
curl -X DELETE http://localhost:9993/security/blacklist/192.168.1.100 \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"

# Complete rate limit reset
curl -X POST http://localhost:9993/security/rate-limit/reset \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"
```

## Application Integration

### Authenticator App Configuration

1. **QR Code Setup:**
   - Get URI from `/security/mfa/setup` endpoint
   - Generate QR code with returned URI
   - Scan with Google Authenticator, Authy, etc.

2. **Manual Setup:**
   ```
   Secret: JBSWY3DPEHPK3PXP
   Algorithm: SHA1
   Digits: 6
   Period: 30 seconds
   ```

3. **Verification:**
   ```bash
   # Test generated code
   curl -X POST http://localhost:9993/security/mfa/verify \
     -H "Authorization: Bearer YOUR_TOKEN" \
     -d '{"code":"123456"}'
   ```

## Troubleshooting

### Common Issues

1. **Token Expired:**
   ```bash
   # Check token expiration
   curl -X GET http://localhost:9993/security/token/validate \
     -H "Authorization: Bearer YOUR_TOKEN"
   ```

2. **MFA Not Working:**
   ```bash
   # Check time synchronization
   # MFA requires precise time sync between server and client
   date
   ```

3. **Rate Limit Reached:**
   ```bash
   # Check current status
   curl -X GET http://localhost:9993/security/rate-limit/status \
     -H "Authorization: Bearer YOUR_TOKEN"
   ```

---

This guide provides practical examples for using all new security features implemented in ZeroTier One.
