import { useEffect, useState } from 'react';
import { Button, Card, Form, Input, InputNumber, message, Radio, Select, Table } from 'antd';
import { tzIdentifiers } from './tzIdentifiers';
import stringify from 'safe-stable-stringify';
import { Typography } from 'antd';
import { getVersionInfo, ipRegex } from './utils';

type MetaValues = {
	chipId: string
}

type MainValues = {
	alignment: string
	rpm: number
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

export default function App() {
	const [messageApi, contextHolder] = message.useMessage();
	const [meta, setMeta] = useState<MetaValues>({
		chipId: ""
	})
	const [mainForm] = Form.useForm()
	const [selectedMode, setSelectedMode] = useState<string>("")
	const [unitStates, setUnitStates] = useState<UnitStates>({
		avrs: [],
		esp: {
			currentMillis: 0
		}
	})
	const [wifiForm] = Form.useForm()
	const [selectedIpAssignment, setSelectedIpAssignment] = useState<string>("dynamic")
	const [miscForm] = Form.useForm()
	const [clock, setClock] = useState<ClockValues>({
		clock: ""
	})

	async function getMetaValues() {
		const res = await fetch("/meta");
		const data = await res.json();
		return data;
	}
	async function getRegisteredMainValues() {
		const res = await fetch('/main');
		const data = await res.json();
		return data;
	}
	async function getRegisteredWifiValues() {
		const res = await fetch('/wifi');
		const data = await res.json();
		return data;
	}
	async function getUnitStates(): Promise<UnitStates> {
		const res = await fetch('/unit');
		const data = await res.json()
		return data
	}
	async function getRegisteredMiscValues() {
		const res = await fetch('/misc')
		const data = await res.json()
		return data
	}
	async function getClockValues() {
		const res = await fetch('/clock')
		const data = await res.json()
		return data
	}
	async function initializeInputs() {
		const metaValues = await getMetaValues()
		setMeta(metaValues)
		const registeredMainValues = await getRegisteredMainValues()
		mainForm.setFieldsValue(registeredMainValues)
		setSelectedMode(registeredMainValues.mode)
		const registeredWifiValues = await getRegisteredWifiValues()
		wifiForm.setFieldsValue(registeredWifiValues)
		const registeredMiscValues = await getRegisteredMiscValues()
		miscForm.setFieldsValue(registeredMiscValues)
		const unitStates = await getUnitStates()
		setUnitStates(unitStates)
		const clockValues = await getClockValues()
		setClock(clockValues)
	}
	useEffect(() => {
		void initializeInputs()
		const intervalHandler = setInterval(async () => {
			const newUnitStates = await getUnitStates()
			setUnitStates(newUnitStates)
			const clock = await getClockValues()
			setClock(clock)
		}, 1000)
		return () => clearInterval(intervalHandler)
	}, [])
	

	async function handleMainFormSubmit(mainFormValues: MainValues) {
		const response = await fetch('/main', {
			method: 'POST',
			body: stringify(mainFormValues),
		})
		if (response.ok) {
			messageApi.success('Successfully updated the main values')
		}
		else {
			messageApi.error('Failed to update the main values')
		}
	}
	async function handleWifiFormSubmit(wifiFormValues: WifiValues) {
		console.log(`wifi post. Input: ${stringify(wifiFormValues)}`)
		const response = await fetch('/wifi', {
			method: 'POST',
			body: stringify(wifiFormValues),
		})
		if (response.ok) {
			messageApi.success('Successfully updated the WiFi values')
		}
		else {
			messageApi.error('Failed to update the WiFi values')
		}
	}

	async function handleOffsetFormSubmit(offsetFormValues: OffsetValues) {
		const response = await fetch('/offset', {
			method: 'POST',
			body: stringify(offsetFormValues),
		});
		setTimeout(async () => {
			// We need to wait a bit before updating the offsets
			// because we need to wait for the server to update the offset of the unit via I2C.
			// Learn more about this situation by reading the comment of `updateOffset` function in the server code.
			const newUnitStates = await getUnitStates()
			setUnitStates(newUnitStates)
		}, 1024)
		if (response.ok) {
			messageApi.success('Successfully updated the offset value')
		} else {
			messageApi.error('Failed to update the offset value')
		}
	}

	const handleOffsetFormSubmitForUnit = (unit: number) => async (offset: number) => {
		await handleOffsetFormSubmit({ unit, offset })
	}

	async function handleMiscFormSubmit(miscFormValues: MiscValues) {
		const response = await fetch('/misc', {
			method: 'POST',
			body: stringify(miscFormValues),
		})
		if (response.ok) {
			messageApi.success('Successfully updated the misc values')
		}
		else {
			messageApi.error('Failed to update the misc values')
		}
	}

	const handleRestart = async () => {
		const response = await fetch('/restart', {
			method: 'POST',
			body: stringify({}) // Gotcha: This empty body is necessary for fetch to send a POST request
		})
		if (response.ok) {
			messageApi.success('Restarting the device')
		}
		else {
			messageApi.error('Failed to restart the device')
		}
	}

	const ipAddressRules = [{ pattern: ipRegex, message: 'Please enter a valid IP address.' }]

	return (
		<div className='mx-4 my-2'>
			{contextHolder}
			<Typography.Title>AfterAI Flaps Web Console</Typography.Title>
			<div className='flex flex-row gap-4'>
				<Typography.Text>Version: {getVersionInfo().semanticVersion}</Typography.Text>
				<Typography.Text>Chip ID: {meta.chipId}</Typography.Text>
			</div>
			<div className='flex flex-row gap-4'>
				<div className="flex flex-row flex-wrap gap-4">
					<Form form={mainForm} onFinish={handleMainFormSubmit}>
						<Card title="Main Settings">
							<div className='flex flex-col gap-4 items-start'>
								<Form.Item name="mode" label="Device Mode">
									<Radio.Group
										options={['text', 'date', 'clock']}
										optionType='button'
										buttonStyle='solid'
										onChange={(e) => setSelectedMode(e.target.value)}
									/>
								</Form.Item>
								<Form.Item name="alignment" label="Alignment">
									<Radio.Group
										options={['left', 'center', 'right']}
										optionType='button'
										buttonStyle='solid'
									/>
								</Form.Item>
								<Form.Item name="rpm" label="RPM">
									<Radio.Group
										options={[8, 9, 10, 11, 12]}
										optionType='button'
										buttonStyle='solid'
									/>
								</Form.Item>
								<Form.Item name="text" label="Text" hidden={selectedMode !== 'text'} >
									<Input
										showCount
										maxLength={unitStates.avrs.length}
									/>
								</Form.Item>
								<Form.Item className='self-center'>
									<Button type="primary" htmlType='submit'>Update</Button>
								</Form.Item>
							</div>
						</Card>
					</Form>

					<Form form={wifiForm} onFinish={handleWifiFormSubmit}>
						<Card title="WiFi Settings">
							<div className='flex flex-col gap-4 items-start'>
								<Form.Item name="ssid" label="SSID">
									<Input type="text" autoComplete="off" />
								</Form.Item>
								<Form.Item name="password" label="Password">
									<Input type="password" id="password" name="password" autoComplete="off" />
								</Form.Item>
								<Form.Item name="ipAssignment" label="IP Assignment">
									<Radio.Group
										options={['dynamic', 'static']}
										optionType='button'
										buttonStyle='solid'
										onChange={(e) => setSelectedIpAssignment(e.target.value)}
									/>
								</Form.Item>
								<Form.Item name="ip" rules={ipAddressRules} label="IP Address" hidden={selectedIpAssignment !== 'static'}>
									<Input type="text" autoComplete="off" />
								</Form.Item>
								<Form.Item name="subnet" rules={ipAddressRules} label="Subnet Mask" hidden={selectedIpAssignment !== 'static'}>
									<Input type="text" autoComplete="off" />
								</Form.Item>
								<Form.Item name="gateway" rules={ipAddressRules} label="Gateway" hidden={selectedIpAssignment !== 'static'}>
									<Input type="text" autoComplete="off" />
								</Form.Item>
								<Form.Item name="dns" rules={ipAddressRules} label="DNS" hidden={selectedIpAssignment !== 'static'}>
									<Input type="text" autoComplete="off" />
								</Form.Item>
								<Form.Item className='self-center'>
									<Button type="primary" htmlType='submit'>Update</Button>
								</Form.Item>
							</div>
						</Card>
					</Form>

					<Form form={miscForm} onFinish={handleMiscFormSubmit}>
						<Card title="Misc Settings">
							<div className='flex flex-col gap-4 items-start'>
								<Form.Item name="timezone" label="Timezone">
									<Select
										className='min-w-64'
										showSearch
										options={tzIdentifiers.map((value) => ({ value, label: value }))}
									/>
									<Typography.Text>Current Time: {clock.clock}</Typography.Text>
								</Form.Item>
								<Form.Item className='self-center'>
									<Button type="primary" htmlType="submit">Update</Button>
								</Form.Item>
							</div>
						</Card>
					</Form>

					<Card title="Operations">
						<div className='flex flex-col gap-4 items-center'>
							<Button type="primary" onClick={handleRestart}>Restart</Button>
						</div>
					</Card>
				</div>

				<Card title="Unit Settings">
					<div className='flex flex-row flex-wrap gap-6 items-center'>
						<Table dataSource={unitStates.avrs}>
							<Table.Column title="Unit" dataIndex="unitAddr" key="unitAddr" />
							<Table.Column title="Offset" dataIndex="offset" key="offset"
								render={(offset, _record, index) => (
									<InputNumber
										className='max-w-20'
										type="number"
										name="offset"
										value={offset}
										min={0}
										max={9999}
										onChange={(value) => {
											setUnitStates((current) => ({
												...current,
												avrs: current.avrs.map((avr, i) => {
													if (i === index) {
														return {
															...avr,
															offset: parseInt(value)
														}
													}
													return avr
												})
											}))
										}}
									/>
								)}
							/>
							<Table.Column title="Update" key="update"
								render={(_update, _record, index) => (
									<Button type="primary" onClick={() => handleOffsetFormSubmitForUnit(index)(unitStates.avrs[index].offset)}>Update</Button>
								)}
							/>
							<Table.Column title="Rotating" dataIndex="rotating" key="rotating"
								render={(rotating) => (
									<Radio checked={rotating} />
								)}
							/>
							<Table.Column title="Last Response (ms ago)" dataIndex="lastResponseAtMillis" key="lastResponseAtMillis"
								render={(lastResponseAtMillis) => (
									<Typography.Text>{unitStates.esp.currentMillis - lastResponseAtMillis}</Typography.Text>
								)}
							/>
						</Table>
					</div>
				</Card>
			</div>
		</div >
	);
}