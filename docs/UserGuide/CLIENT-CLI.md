\page client_cli Interacting via Client CLI

# Client CLI
Resource-tuner provides a minimal CLI to interact with the server. This is provided to help with development and debugging purposes.

## Usage Examples

### 1. Send a Tune Request
```bash
/usr/bin/urmCli --tune --duration <> --priority <> --num <> --res <>
```
Where:
- `duration`: Duration in milliseconds for the tune request
- `priority`: Priority level for the tune request (HIGH: 0 or LOW: 1)
- `num`: Number of resources
- `res`: List of resource ResCode, ResInfo (optional) and Values to be tuned as part of this request

Example:
```bash
# Single Resource in a Request
/usr/bin/urmCli --tune --duration 5000 --priority 0 --num 1 --res "65536:700"

# Multiple Resources in single Request
/usr/bin/urmCli --tune --duration 4400 --priority 1 --num 2 --res "0x80030000:700,0x80040001:155667"

# Multi-Valued Resource
/usr/bin/urmCli --tune --duration 9500 --priority 0 --num 1 --res "0x00090002:0,0,1,3,5"

# Specifying ResInfo (useful for Core and Cluster type Resources)
/usr/bin/urmCli --tune --duration 5000 --priority 0 --num 1 --res "0x00040000#0x00000100:1620438"

# Everything at once
/usr/bin/urmCli --tune --duration 6500 --priority 0 --num 2 --res "0x00030000:800;0x00040011#0x00000101:50000,100000"
```

### 2. Send an Untune Request
```bash
/usr/bin/urmCli --untune --handle <>
```
Where:
- `handle`: Handle of the previously issued tune request, which needs to be untuned

Example:
```bash
/usr/bin/urmCli --untune --handle 50
```

### 3. Send a Retune Request
```bash
/usr/bin/urmCli --retune --handle <> --duration <>
```
Where:
- `handle`: Handle of the previously issued tune request, which needs to be retuned
- `duration`: The new duration in milliseconds for the tune request

Example:
```bash
/usr/bin/urmCli --retune --handle 7 --duration 8000
```

### 4. Send a getProp Request

```bash
/usr/bin/urmCli --getProp --key <>
```
Where:
- `key`: The Prop name of which the corresponding value needs to be fetched

Example:
```bash
/usr/bin/urmCli --getProp --key "urm.logging.level"
```

### 5. Send a tuneSignal Request

```bash
/usr/bin/urmCli --signal --scode <>
```
Where:
- `key`: The Prop name of which the corresponding value needs to be fetched

Example:
```bash
/usr/bin/urmCli --signal --scode "0x00fe0ab1"
```

<div style="page-break-after: always;"></div>
