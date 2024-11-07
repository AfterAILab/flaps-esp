# AfterAI Flaps ESP 1.1 API Document

## Overview

REST API for controlling AfterAI Flaps display units. Base URL:
`http://<device-ip>`

## Authentication

All endpoints are currently unauthenticated.

## Common Response Codes

- `200` - Success
- `400` - Bad Request (invalid parameters)
- `500` - Internal Server Error

## Endpoints

### `GET /`

Returns the web interface HTML.

### `GET /meta`

Returns device information.

**Response:**

```
{
	"chipId": "string" // Unique identifier of the ESP chip
}
```

### `GET /main`

Returns the current display configuration.

**Response:**

```
{
	"alignment": "string", // Text alignment ("left", "center", "right")
	"rpm": "number", // Rotation speed in RPM (1-12)
	"mode": "string", // Display mode ("text", "date", "clock")
	"numUnits": "number", // Number of connected display units (0-128)
	"text": "string" // Text to display (meaningful only if mode="text")
}
```

### `POST /main`

Updates display configuration.

**Request:**

```
{
	"alignment": "string", // Optional: Text alignment
	"rpm": "number", // Optional: Rotation speed
	"mode": "string", // Optional: Display mode
	"numUnits": "number", // Optional: Number of units
	"text": "string" // Optional: Text to display (required if mode="text")
}
```

**Response:** Same as `GET /main`

### `GET /wifi`

Returns current WiFi configuration.

**Response:**

```
{
	"ssid": "string", // Network name
	"ipAssignment": "string", // "dynamic" or "static"
	"ip": "string", // Static IP (if applicable)
	"subnet": "string", // Subnet mask (if static)
	"gateway": "string", // Gateway address (if static)
	"dns": "string" // DNS server (if static)
}
```

### `POST /wifi`

Updates WiFi configuration.

**Request:**

```
{
	"ssid": "string", // Network name
	"password": "string", // Network password
	"ipAssignment": "string", // "dynamic" or "static"
	"ip": "string", // Required if static
	"subnet": "string", // Required if static
	"gateway": "string", // Required if static
	"dns": "string" // Required if static
}
```

**Response:** Same as `GET /wifi`

### `GET /misc`

Returns miscellaneous settings.

**Response:**

```
{
	"timezone": "string", // IANA timezone
	"numI2CBusStuck": "number", // Number of I2C bus errors
	"lastI2CBusStuckAgoInMillis": "number" // Milliseconds since last I2C error
}
```

### `POST /misc`

Updates miscellaneous settings.

**Request:**

```
{
	"timezone": "string" // IANA timezone identifier
}
```

**Response:** Same as `GET /misc`

### `GET /clock`

Returns current time.

**Response:**

```
{
	"clock": "string" // Current time in HH:MM format
}
```

### `GET /offset`

Returns offset values for all units.

**Response:**

```
[
	"number" // Array of offset values
]
```

### `GET /unit`

Returns status of all display units.

**Response:**

```
{
	"avrs": [
		{
		"unitAddr": "number", // Unit address
		"rotating": "boolean", // Whether unit is rotating
		"offset": "number", // Current offset value
		"magneticZeroPositionLetterIndex": "number", // Zero position index
		"lastResponseAtMillis": "number" // Last response timestamp
		}
	],
	"esp": {
		"currentMillis": "number" // Current ESP timestamp
	}
}
```

### `POST /unit`

Updates unit configuration.

**Request:**

```
[
	{
		"offset": "number", // New offset value
		"magneticZeroPositionLetterIndex": "number" // New zero position index
	}
]
```

**Response:** Same as `GET /unit`

### `POST /restart`

Triggers ESP chip restart.

**Response:** No content (device will restart)

## Operation Modes

The device supports three operation modes:

- `STA` (0) - Station mode (connects to WiFi)
- `AP` (1) - Access Point mode (creates WiFi network)
- `OFF` (2) - Offline mode (no WiFi)

## Rate Limiting

No rate limiting is currently implemented.
