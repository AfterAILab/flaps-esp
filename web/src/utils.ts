export function getVersionInfo() {
    return {
        semanticVersion: "1.0.0-beta",
    }
}

export const ipRegex = /^(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$/;

export function convertMillisToConvenientString(lastResponseBusStuckAtMillis: number) {
    if (lastResponseBusStuckAtMillis < 1000) {
        return `${lastResponseBusStuckAtMillis} ms`;
    }
    const seconds = Math.floor(lastResponseBusStuckAtMillis / 1000);
    if (seconds < 60) {
        return `${seconds} s`;
    }
    const minutes = Math.floor(seconds / 60);
    if (minutes < 60) {
        return `${minutes} m`;
    }
    const hours = Math.floor(minutes / 60);
    return `${hours} h`;
}