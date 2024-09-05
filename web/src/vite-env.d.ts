/// <reference types="vite/client" />

type MetaValues = {
	chipId: string
}

type MainValues = {
	alignment: string
	rpm: number
	numUnits: number
	mode: string
}

type WifiValues = {
	ssid: string
	password: string
}

type MiscValues = {
	timezone: string
}

type OffsetValues = {
	unit: number
	offset: number
}

type AvrState = {
	unitAddr: number
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
