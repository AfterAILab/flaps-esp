import { useEffect, useState } from 'react';
import { Button, Card, Form, Input, InputNumber, message, Popover, Radio, Select, Table } from 'antd';
import { tzIdentifiers } from './tzIdentifiers';
import stringify from 'safe-stable-stringify';
import { Typography } from 'antd';
import { convertMillisToConvenientString, getVersionInfo, ipRegex, offsetGuideTableColumns, offsetGuideTableData } from './utils';

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
	const [getAndSetUnitStatesIntervalHandler, setGetAndSetUnitStatesIntervalHandler] = useState<number | undefined>(undefined)
	const [getAndSetClockIntervalHandler, setGetAndSetClockIntervalHandler] = useState<number | undefined>(undefined)
	const [unitsScan, setUnitsScan] = useState('per second')

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
		// As I could not find a way to make Arduino_JSON generate an empty array string, I have to do this workaround.
		// TODO: Find a way to make Arduino_JSON generate an empty array string.
		if (data.avrs === undefined || data.avrs === null) {
			return {
				...data,
				avrs: []
			}
		}
		console.log(`getUnitStates: ${stringify(data)}`)
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
		const unitStatesHandeler = setInterval(async () => {
			const newUnitStates = await getUnitStates()
			setUnitStates(newUnitStates)
		}, 1000)
		setGetAndSetUnitStatesIntervalHandler(unitStatesHandeler)
		const clockHandler = setInterval(async () => {
			const clock = await getClockValues()
			setClock(clock)
		}, 1000)
		setGetAndSetClockIntervalHandler(clockHandler)
		return () => {
			clearInterval(getAndSetUnitStatesIntervalHandler)
			clearInterval(getAndSetClockIntervalHandler)
		}
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

	async function handleUnitFormSubmit() {
		const response = await fetch('/unit', {
			method: 'POST',
			body: stringify(unitStates.avrs),
		});
		setTimeout(async () => {
			// We need to wait a bit before updating the offsets
			// because we need to wait for the server to update the offset of the unit via I2C.
			// Learn more about this situation by reading the comment of `updateOffset` function in the server code.
			const newUnitStates = await getUnitStates()
			setUnitStates(newUnitStates)
		}, 1000)
		if (response.ok) {
			messageApi.success('Successfully updated the offset value')
		} else {
			messageApi.error('Failed to update the offset value')
		}
	}

	async function handleMiscFormSubmit(miscFormValues: MiscValues) {
		console.log(`misc post. Input: ${stringify(miscFormValues)}`)
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
				<div className="flex flex-row flex-wrap gap-4 items-start">
					<Form form={mainForm} onFinish={handleMainFormSubmit}>
						<Card title="Main Settings">
							<div className='flex flex-col gap-4 items-start'>
								<Form.Item name="numUnits" label="Number of Units">
									<InputNumber min={0} max={128} />
								</Form.Item>
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
									<InputNumber min={1} max={12} />
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
						<Card title="Wi-Fi Settings">
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
								<div className='flex flex-col items-start'>
									<Form.Item name="timezone" label="Timezone">
										<Select
											className='min-w-64'
											showSearch
											options={tzIdentifiers.map((value) => ({ value, label: value }))}
										/>
									</Form.Item>
									<Typography.Text>
										Current Time
										<Popover
											placement='top'
											trigger='click'
											content={<span>Correct time is available if Leader is connected to the Internet.</span>}
										>
											<Button type="link" shape="circle" size="small">?</Button>
										</Popover>
										: {clock.clock}
									</Typography.Text>
								</div>
								<div className='flex flex-col items-start gap-1'>
									<Form.Item name="numI2CBusStuck" label="Number of I2C Bus Stuck" className='!mb-0'>
										<InputNumber variant='borderless' disabled />
									</Form.Item>
									<Form.Item name="lastI2CBusStuckAgoInMillis" label="Last I2C Bus Stuck (ago)">
										<InputNumber
											variant='borderless'
											disabled
											formatter={(value) => convertMillisToConvenientString(parseInt(`${value}`, 10))}
										/>
									</Form.Item>
								</div>
								<Form.Item className='self-center'>
									<Button type="primary" htmlType="submit">Update</Button>
								</Form.Item>
							</div>
						</Card>
					</Form>

					<Card title="Operations">
						<div className='flex flex-col gap-4 items-center'>
							<Button type="primary" onClick={handleRestart}>Restart</Button>
							<Button type="primary" onClick={async () => {
								for (const l of `AfterAI 2024`.split('')) {
									// create an array of length
									const text = Array(mainForm.getFieldValue('numUnits')).fill(l).join('')
									mainForm.setFieldsValue({
										mode: 'text',
										text
									})
									setSelectedMode('text')
									await handleMainFormSubmit({
										...mainForm.getFieldsValue(true),
										text,
										mode: 'text'
									})
									await new Promise((resolve) => setTimeout(resolve, 5000))
								}
							}}>Text Tour</Button>
						</div>
					</Card>
				</div>

				<Card title="Unit Settings">
					<div className='flex flex-row flex-wrap gap-6 items-start'>
						<Form>
							<Form.Item label="Units Scan">
								<Radio.Group
									options={['stop', 'per second', 'per 10 seconds']}
									optionType='button'
									buttonStyle='solid'
									value={unitsScan}
									onChange={(e) => {
										clearInterval(getAndSetUnitStatesIntervalHandler)
										setGetAndSetUnitStatesIntervalHandler(undefined)
										setUnitsScan(e.target.value)
										switch (e.target.value) {
											case 'stop':
												break
											case 'per second': {
												const unitStatesHandeler = setInterval(async () => {
													const newUnitStates = await getUnitStates()
													setUnitStates(newUnitStates)
												}, 1000)
												setGetAndSetUnitStatesIntervalHandler(unitStatesHandeler)
												break
											}
											case 'per 10 seconds': {
												const unitStatesHandeler10 = setInterval(async () => {
													const newUnitStates = await getUnitStates()
													setUnitStates(newUnitStates)
												}, 10000)
												setGetAndSetUnitStatesIntervalHandler(unitStatesHandeler10)
												break
											}
											default:
												throw new Error('Invalid unitsScan value')
										}
									}}
								/>
							</Form.Item>
						</Form>
						<Form>
							<Button type="primary" onClick={async () => {
								await handleUnitFormSubmit()
								setUnitsScan('per second')
								clearInterval(getAndSetUnitStatesIntervalHandler)
								const unitStatesHandeler = setInterval(async () => {
									const newUnitStates = await getUnitStates()
									setUnitStates(newUnitStates)
								}, 1000)
								setGetAndSetUnitStatesIntervalHandler(unitStatesHandeler)
							}}>Update</Button>
							<Table dataSource={unitStates.avrs}>
								<Table.Column title="Unit" dataIndex="unitAddr" key="unitAddr" />
								<Table.Column
									title="Magnetic Zero Position Letter"
									dataIndex="magneticZeroPositionLetterIndex"
									key="magneticZeroPositionLetterIndex"
									render={(magneticZeroPositionLetterIndex, record) => {
										return (
											<Select
												className='w-24'
												options={
													offsetGuideTableData.map(({ key, zeroOffsetStoppingCharacter }) => ({
														value: parseInt(key),
														label: zeroOffsetStoppingCharacter
													}))}
												value={magneticZeroPositionLetterIndex}
												onChange={(value) => {
													// Workaround to keep the editing data in the input field
													setUnitsScan('stop')
													clearInterval(getAndSetUnitStatesIntervalHandler)
													setGetAndSetUnitStatesIntervalHandler(undefined)
													const suggestedOffset = offsetGuideTableData.find(({ key }) => key === `${value}`)?.suggestedOffset ?? '0'

													setUnitStates((current) => ({
														...current,
														avrs: current.avrs.map((avr, i) => {
															if (i === record.unitAddr) {
																return {
																	...avr,
																	offset: parseInt(suggestedOffset),
																	magneticZeroPositionLetterIndex: value
																}
															}
															return avr
														})
													}))
												}}
											/>)
									}}
								/>
								<Table.Column
									title={
										<span>
											Offset
											<Popover
												placement='right'
												trigger='click'
												content={
													<Table
														size='small'
														dataSource={offsetGuideTableData}
														columns={offsetGuideTableColumns} />
												}
											>
												<Button type="link" shape="circle" size="small">?</Button>
											</Popover>
										</span>}
									dataIndex="offset" key="offset"
									render={(offset, record) => (
										<InputNumber
											className='max-w-20'
											type="number"
											name="offset"
											value={offset}
											min={0}
											max={2038}
											onChange={(value) => {
												// Workaround to keep the editing data in the input field
												setUnitsScan('stop')
												clearInterval(getAndSetUnitStatesIntervalHandler)
												setGetAndSetUnitStatesIntervalHandler(undefined)

												setUnitStates((current) => ({
													...current,
													avrs: current.avrs.map((avr, i) => {
														if (i === record.unitAddr) {
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
								<Table.Column title="Rotating" dataIndex="rotating" key="rotating"
									render={(rotating) => (
										<Radio checked={rotating} />
									)}
								/>
								<Table.Column title="Last Response (ago)" dataIndex="lastResponseAtMillis" key="lastResponseAtMillis"
									render={(lastResponseAtMillis) => (
										<Typography.Text>{convertMillisToConvenientString(unitStates.esp.currentMillis - lastResponseAtMillis)}</Typography.Text>
									)}
								/>
							</Table>
						</Form>
					</div>
				</Card>
			</div>
		</div >
	);
}