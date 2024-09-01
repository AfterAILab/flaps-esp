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

export default function App() {
	const [messageApi, contextHolder] = message.useMessage();
	const [meta, setMeta] = useState<MetaValues>({
		chipId: ""
	})
	const [mainForm] = Form.useForm()
	const [selectedMode, setSelectedMode] = useState<string>("")
	const [offsets, setOffsets] = useState<number[]>([])
	const [wifiForm] = Form.useForm()
	const [selectedIpAssignment, setSelectedIpAssignment] = useState<string>("dynamic")
	const [miscForm] = Form.useForm()

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
	async function getRegisteredOffsets() {
		const res = await fetch('/offset')
		const data = await res.json()
		return data
	}
	async function getRegisteredMiscValues() {
		const res = await fetch('/misc')
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
		const registeredOffsets = await getRegisteredOffsets()
		setOffsets(registeredOffsets)
	}
	useEffect(() => {
		void initializeInputs()
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
			const registeredOffsets = await getRegisteredOffsets()
			setOffsets(registeredOffsets)
		}, 500)
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

	const ipAddressRules = [{pattern: ipRegex, message: 'Please enter a valid IP address.'}]

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
									<Input showCount maxLength={offsets.length}/>
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
								</Form.Item>
								<Form.Item className='self-center'>
									<Button type="primary" htmlType="submit">Update</Button>
								</Form.Item>
							</div>
						</Card>
					</Form>
				</div>

				<Card title="Offset Settings">
					<div className='flex flex-row flex-wrap gap-6 items-center'>
						<Table dataSource={offsets.map((offset, index) => ({ unit: index, offset }))}>
							<Table.Column title="Unit" dataIndex="unit" key="unit" />
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
										const newOffsets = [...offsets]
										newOffsets[index] = parseInt(value)
										setOffsets(newOffsets)
										console.log(`newOffsets: ${newOffsets}`)
									}}
								/>
								)}
							/>
							<Table.Column title="Update" key="update"
								render={(_text, _record, index) => (
									<Button type="primary" onClick={() => handleOffsetFormSubmitForUnit(index)(offsets[index])}>Update</Button>
								)}
							/>
						</Table>
					</div>
				</Card>
			</div>
		</div >
	);
}