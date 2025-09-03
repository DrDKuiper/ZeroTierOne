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

# ZeroTier One - Security API Usage Guide

## Practical Guide to New Endpoints

This guide demonstrates how to use the new security endpoints implemented in ZeroTier One.

## Authentication

### 1. Generating a JWT Token

**Endpoint:** `POST /security/token`

```bash
curl -X POST http://localhost:9993/security/token \
  -H "x-zt1-auth: <your_admin_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "generate",
    "scope": "api:write",
    "expires_in": 7200
  }'
```

**Response:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "scope": "api:write",
  "expires_in": 7200
}
```

### 2. Using JWT Token for Authentication

```bash
curl -X GET http://localhost:9993/status \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

### 3. Revoking a Token

```bash
curl -X POST http://localhost:9993/security/token \
  -H "x-zt1-auth: <your_admin_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "revoke",
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
  }'
```

## Multi-Factor Authentication (MFA)

### 1. Setting up MFA

**Endpoint:** `GET /security/mfa/setup`

```bash
curl -X GET http://localhost:9993/security/mfa/setup \
  -H "x-zt1-auth: <your_admin_token>" \
  -H "x-client-id: admin-workstation"
```

**Response:**
```json
{
  "qr_code": "otpauth://totp/ZeroTier:admin-workstation?secret=JBSWY3DPEHPK3PXP&issuer=ZeroTier",
  "secret": "JBSWY3DPEHPK3PXP",
  "backup_codes": ["123456", "789012", "345678"]
}
```

### 2. Checking MFA Status

```bash
curl -X GET http://localhost:9993/security/mfa/status \
  -H "x-zt1-auth: <your_admin_token>" \
  -H "x-client-id: admin-workstation"
```

### 3. Verifying MFA Code

```bash
curl -X POST http://localhost:9993/security/mfa/verify \
  -H "x-zt1-auth: <your_admin_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "challenge_id": "challenge_123",
    "code": "123456"
  }'
```

### 4. Accessing MFA-Required Endpoint

```bash
curl -X GET http://localhost:9993/controller/network \
  -H "Authorization: Bearer <jwt_token>" \
  -H "x-mfa-token: 123456" \
  -H "x-client-id: admin-workstation"
```

## Bandwidth Control

### 1. Configuring Network-Level Limits

**Endpoint:** `POST /security/bandwidth/control`

```bash
curl -X POST http://localhost:9993/security/bandwidth/control \
  -H "Authorization: Bearer <jwt_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "network_id": "8056c2e21c000001",
    "upload_limit": 10485760,
    "download_limit": 52428800,
    "burst_allowance": 2097152,
    "qos_class": 2
  }'
```

**Response:**
```json
{
  "network_id": "8056c2e21c000001",
  "bandwidth_configured": true
}
```

### 2. Configuring Peer-Level Limits

```bash
curl -X POST http://localhost:9993/security/bandwidth/control \
  -H "Authorization: Bearer <jwt_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "peer_id": "8056c2e21c",
    "upload_limit": 5242880,
    "download_limit": 26214400,
    "burst_allowance": 1048576,
    "qos_class": 1
  }'
```

### 3. Viewing Bandwidth Statistics

```bash
curl -X GET http://localhost:9993/security/bandwidth/stats \
  -H "Authorization: Bearer <jwt_token>"
```

**Response:**
```json
{
  "bandwidth_controller": {
    "enabled": true,
    "active_limits": 5,
    "total_traffic": "1.2 GB",
    "qos_classes": ["Critical", "High", "Normal", "Low"]
  },
  "network_stats": {
    "8056c2e21c000001": {
      "upload_bytes": 1048576,
      "download_bytes": 5242880,
      "upload_packets": 1000,
      "download_packets": 5000,
      "dropped_packets": 10
    }
  }
}
```

## QoS Policy Configuration

### Defining Bandwidth Policy

```bash
curl -X POST http://localhost:9993/security/bandwidth/policy \
  -H "Authorization: Bearer <jwt_token>" \
  -H "Content-Type: application/json" \
  -d '{
    "policy_name": "enterprise_qos",
    "default_class": "Normal",
    "classes": {
      "Critical": {
        "priority": 1,
        "guaranteed_bandwidth": "50%",
        "max_bandwidth": "100%"
      },
      "High": {
        "priority": 2,
        "guaranteed_bandwidth": "30%",
        "max_bandwidth": "80%"
      },
      "Normal": {
        "priority": 3,
        "guaranteed_bandwidth": "15%",
        "max_bandwidth": "60%"
      },
      "Low": {
        "priority": 4,
        "guaranteed_bandwidth": "5%",
        "max_bandwidth": "20%"
      }
    }
  }'
```

## Monitoring and Auditing

### 1. Viewing Security Overview

**Endpoint:** `GET /security/overview`

```bash
curl -X GET http://localhost:9993/security/overview \
  -H "Authorization: Bearer <jwt_token>"
```

**Response:**
```json
{
  "secure_api": {
    "enabled": true,
    "jwt_authentication": true,
    "rate_limiting": true,
    "reputation_tracking": true
  },
  "mfa": {
    "enabled": true,
    "totp_support": true,
    "challenge_based": true
  },
  "bandwidth_control": {
    "enabled": true,
    "qos_classes": 4,
    "traffic_shaping": true,
    "per_network_limits": true,
    "per_peer_limits": true
  },
  "security_metrics": {
    "total_security_events": 156,
    "active_jwt_tokens": 5,
    "rate_limited_ips": 2,
    "mfa_enabled_clients": 3
  }
}
```

### 2. Accessing Audit Logs

```bash
curl -X GET http://localhost:9993/security/audit \
  -H "Authorization: Bearer <jwt_token>"
```

**Response:**
```json
{
  "audit_logs": [
    {
      "timestamp": "2024-01-15T10:30:00Z",
      "event_type": "auth_success",
      "description": "JWT authentication successful",
      "severity": "INFO",
      "client_ip": "192.168.1.100"
    },
    {
      "timestamp": "2024-01-15T10:29:45Z",
      "event_type": "mfa_verified",
      "description": "MFA verification successful",
      "severity": "INFO",
      "client_ip": "192.168.1.100"
    }
  ]
}
```

## Authorization Scopes

### Scope Hierarchy

1. **`api:read`** - Basic status and configuration reading
2. **`api:write`** - Network configuration modification
3. **`controller:admin`** - Network controller administration
4. **`security:read`** - Security information reading
5. **`security:admin`** - Complete security administration

### Endpoint Mapping by Scope

```
api:read:
  - GET /status
  - GET /network
  - GET /peer

api:write:
  - POST /network
  - DELETE /network
  - POST /peer

controller:admin:
  - /controller/* (all controller endpoints)

security:read:
  - GET /security/overview
  - GET /security/audit
  - GET /security/bandwidth/stats

security:admin:
  - POST /security/token
  - POST /security/mfa/*
  - POST /security/bandwidth/*
```

## Error Handling

### HTTP Status Codes

- **200 OK** - Successful operation
- **400 Bad Request** - Invalid request data
- **401 Unauthorized** - Authentication failure
- **403 Forbidden** - Insufficient scope
- **429 Too Many Requests** - Rate limit exceeded
- **500 Internal Server Error** - Internal server error

### Common Error Responses

```json
{
  "error": "Rate limit exceeded",
  "retry_after": 60
}

{
  "error": "MFA token required",
  "mfa_required": true
}

{
  "error": "Invalid scope for this operation",
  "required_scope": "security:admin"
}
```

## Legacy API Migration

### Compatibility

The new API maintains full compatibility with existing API:

```bash
# Legacy API (continues to work)
curl -X GET http://localhost:9993/status \
  -H "x-zt1-auth: <legacy_token>"

# New JWT API
curl -X GET http://localhost:9993/status \
  -H "Authorization: Bearer <jwt_token>"
```

### Migration Recommendations

1. **Phase 1**: Continue using legacy tokens for normal operations
2. **Phase 2**: Implement JWT for new clients/applications
3. **Phase 3**: Enable MFA for administrative operations
4. **Phase 4**: Configure rate limiting and bandwidth control
5. **Phase 5**: Migrate completely to JWT

### Migration Script Example

```bash
#!/bin/bash

# 1. Generate JWT token for migration
JWT_RESPONSE=$(curl -s -X POST http://localhost:9993/security/token \
  -H "x-zt1-auth: $LEGACY_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"action": "generate", "scope": "api:write", "expires_in": 86400}')

JWT_TOKEN=$(echo $JWT_RESPONSE | jq -r '.token')

# 2. Configure MFA for admin
curl -X GET http://localhost:9993/security/mfa/setup \
  -H "Authorization: Bearer $JWT_TOKEN" \
  -H "x-client-id: migration-script"

# 3. Configure bandwidth limits
curl -X POST http://localhost:9993/security/bandwidth/control \
  -H "Authorization: Bearer $JWT_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "network_id": "YOUR_NETWORK_ID",
    "upload_limit": 10485760,
    "download_limit": 52428800
  }'

echo "Migration completed successfully"
```

## Troubleshooting

### Common Issues

1. **Active Rate Limiting**
   - Check `/security/overview` for limited IPs
   - Wait for cooldown period
   - Consider whitelisting trusted IPs

2. **MFA Not Working**
   - Check clock synchronization
   - Use backup codes if available
   - Reconfigure MFA if necessary

3. **Invalid JWT Token**
   - Check token expiration
   - Generate new token if needed
   - Confirm adequate scope

4. **Bandwidth Control Not Applied**
   - Check QoS configuration
   - Confirm controller is active
   - Monitor audit logs for errors

## Professional Change Summary

### Security Enhancements Implemented

#### 1. **JWT Authentication System**
- **Implementation**: Full JWT token-based authentication with HS256 algorithm
- **Business Impact**: Enhanced security with granular scope-based access control
- **Technical Details**: Tokens with configurable expiration, revocation capabilities, and scope validation
- **Compliance**: Supports enterprise security standards and audit requirements

#### 2. **Multi-Factor Authentication (MFA)**
- **Implementation**: TOTP-based MFA compatible with Google Authenticator and similar apps
- **Business Impact**: Additional security layer for critical administrative operations
- **Technical Details**: Challenge/response system with backup codes and client-specific configuration
- **Compliance**: Meets regulatory requirements for administrative access protection

#### 3. **Advanced Rate Limiting**
- **Implementation**: Adaptive rate limiting with client reputation tracking
- **Business Impact**: Protection against DoS attacks and abuse while maintaining service quality
- **Technical Details**: Reputation-based scoring system with configurable thresholds
- **Compliance**: Enhances service availability and prevents unauthorized access attempts

#### 4. **Bandwidth Control & QoS**
- **Implementation**: Comprehensive traffic shaping with 4-tier QoS classification
- **Business Impact**: Guaranteed service quality and resource optimization
- **Technical Details**: Per-network and per-peer bandwidth limits with burst allowance
- **Compliance**: Supports SLA enforcement and resource management policies

#### 5. **Security Auditing**
- **Implementation**: Comprehensive audit logging with multiple severity levels
- **Business Impact**: Full visibility into security events for compliance and monitoring
- **Technical Details**: Structured JSON logs with timestamps, event types, and client tracking
- **Compliance**: Supports regulatory audit requirements and incident investigation

### Integration Architecture

#### **Backward Compatibility**
- **Approach**: Hybrid authentication system maintaining full legacy API compatibility
- **Implementation**: Gradual migration path with parallel authentication methods
- **Risk Mitigation**: Zero-downtime deployment with fallback mechanisms

#### **Performance Optimization**
- **Monitoring**: OpenTelemetry integration for distributed tracing and metrics
- **Scalability**: Efficient memory management and configurable resource limits
- **Reliability**: Error handling and graceful degradation under load

### Deployment Considerations

#### **Enterprise Readiness**
- **Security**: Multiple authentication layers with configurable policies
- **Monitoring**: Real-time security metrics and audit trail capabilities
- **Management**: RESTful API for all security configurations and monitoring

#### **Operational Excellence**
- **Documentation**: Comprehensive implementation and usage guides
- **Testing**: Error-free compilation and integration with existing codebase
- **Maintenance**: Modular design for easy updates and configuration changes

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
