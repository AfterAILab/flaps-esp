/// <reference types="vite/client" />

type MetaValues = {
	chipId: string
}

type MainValues = {
	alignment: string
	rpm: number
	numUnits: number
	mode: string
	text: string
}

type WifiValues = {
	ssid: string
	password: string
}

type MiscValues = {
	timezone: string
	numI2CBusStuck: number
	lastI2CBusStuckAgoInMillis: number
}

type UnitValues = {
	unitAddr: number
	magneticZeroPositionLetterIndex: number
	offset: number
}

type AvrState = {
	unitAddr: number
	magneticZeroPositionLetterIndex: number
	offset: number
	rotating: boolean
	lastResponseAtMillis: number
}

type UnitStates = {
	avrs: AvrState[]
	esp: {
		currentMillis: number
	}
}

type ClockValues = {
	clock: string
}
