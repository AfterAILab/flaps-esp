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

type OffsetGuideTableData = {
    key: string,
    zeroOffsetStoppingCharacter: string,
    suggestedOffset: string,
}

export const offsetGuideTableColumns = [
    {
        title: "Zero-offset stopping character",
        dataIndex: "zeroOffsetStoppingCharacter",
        key: "zeroOffsetStoppingCharacter",
        /* TODO: Implement filter
        filterSearch: (input: string, record: ColumnFilterItem) => {
            const recordValue = record.value
            return recordValue !== undefined && typeof recordValue === 'string' && recordValue.toUpperCase().includes(input.toUpperCase())
        },
        */
    },
    {
        title: "Suggested offset",
        dataIndex: "suggestedOffset",
        key: "suggestedOffset",
    }
]

export const offsetGuideTableData: OffsetGuideTableData[] = [
    {
        key: '0',
        zeroOffsetStoppingCharacter: '(space)',
        suggestedOffset: '0'
    },
    {
        key: '1',
        zeroOffsetStoppingCharacter: 'A',
        suggestedOffset: '1993'
    },
    {
        key: '2',
        zeroOffsetStoppingCharacter: 'B',
        suggestedOffset: '1947'
    },
    {
        key: '3',
        zeroOffsetStoppingCharacter: 'C',
        suggestedOffset: '1902'
    },
    {
        key: '4',
        zeroOffsetStoppingCharacter: 'D',
        suggestedOffset: '1857'
    },
    {
        key: '5',
        zeroOffsetStoppingCharacter: 'E',
        suggestedOffset: '1812'
    },
    {
        key: '6',
        zeroOffsetStoppingCharacter: 'F',
        suggestedOffset: '1766'
    },
    {
        key: '7',
        zeroOffsetStoppingCharacter: 'G',
        suggestedOffset: '1721'
    },
    {
        key: '8',
        zeroOffsetStoppingCharacter: 'H',
        suggestedOffset: '1676'
    },
    {
        key: '9',
        zeroOffsetStoppingCharacter: 'I',
        suggestedOffset: '1630'
    },
    {
        key: '10',
        zeroOffsetStoppingCharacter: 'J',
        suggestedOffset: '1585'
    },
    {
        key: '11',
        zeroOffsetStoppingCharacter: 'K',
        suggestedOffset: '1540'
    },
    {
        key: '12',
        zeroOffsetStoppingCharacter: 'L',
        suggestedOffset: '1495'
    },
    {
        key: '13',
        zeroOffsetStoppingCharacter: 'M',
        suggestedOffset: '1449'
    },
    {
        key: '14',
        zeroOffsetStoppingCharacter: 'N',
        suggestedOffset: '1404'
    },
    {
        key: '15',
        zeroOffsetStoppingCharacter: 'O',
        suggestedOffset: '1359'
    },
    {
        key: '16',
        zeroOffsetStoppingCharacter: 'P',
        suggestedOffset: '1313'
    },
    {
        key: '17',
        zeroOffsetStoppingCharacter: 'Q',
        suggestedOffset: '1268'
    },
    {
        key: '18',
        zeroOffsetStoppingCharacter: 'R',
        suggestedOffset: '1223'
    },
    {
        key: '19',
        zeroOffsetStoppingCharacter: 'S',
        suggestedOffset: '1178'
    },
    {
        key: '20',
        zeroOffsetStoppingCharacter: 'T',
        suggestedOffset: '1132'
    },
    {
        key: '21',
        zeroOffsetStoppingCharacter: 'U',
        suggestedOffset: '1087'
    },
    {
        key: '22',
        zeroOffsetStoppingCharacter: 'V',
        suggestedOffset: '1042'
    },
    {
        key: '23',
        zeroOffsetStoppingCharacter: 'W',
        suggestedOffset: '996'
    },
    {
        key: '24',
        zeroOffsetStoppingCharacter: 'X',
        suggestedOffset: '951'
    },
    {
        key: '25',
        zeroOffsetStoppingCharacter: 'Y',
        suggestedOffset: '906'
    },
    {
        key: '26',
        zeroOffsetStoppingCharacter: 'Z',
        suggestedOffset: '860'
    },
    {
        key: '27',
        zeroOffsetStoppingCharacter: '$',
        suggestedOffset: '815'
    },
    {
        key: '28',
        zeroOffsetStoppingCharacter: '&',
        suggestedOffset: '770'
    },
    {
        key: '29',
        zeroOffsetStoppingCharacter: '#',
        suggestedOffset: '725'
    },
    {
        key: '30',
        zeroOffsetStoppingCharacter: '0',
        suggestedOffset: '679'
    },
    {
        key: '31',
        zeroOffsetStoppingCharacter: '1',
        suggestedOffset: '634'
    },
    {
        key: '32',
        zeroOffsetStoppingCharacter: '2',
        suggestedOffset: '589'
    },
    {
        key: '33',
        zeroOffsetStoppingCharacter: '3',
        suggestedOffset: '543'
    },
    {
        key: '34',
        zeroOffsetStoppingCharacter: '4',
        suggestedOffset: '498'
    },
    {
        key: '35',
        zeroOffsetStoppingCharacter: '5',
        suggestedOffset: '453'
    },
    {
        key: '36',
        zeroOffsetStoppingCharacter: '6',
        suggestedOffset: '408'
    },
    {
        key: '37',
        zeroOffsetStoppingCharacter: '7',
        suggestedOffset: '362'
    },
    {
        key: '38',
        zeroOffsetStoppingCharacter: '8',
        suggestedOffset: '317'
    },
    {
        key: '39',
        zeroOffsetStoppingCharacter: '9',
        suggestedOffset: '272'
    },
    {
        key: '40',
        zeroOffsetStoppingCharacter: ':',
        suggestedOffset: '226'
    },
    {
        key: '41',
        zeroOffsetStoppingCharacter: '.',
        suggestedOffset: '181'
    },
    {
        key: '42',
        zeroOffsetStoppingCharacter: '-',
        suggestedOffset: '136'
    },
    {
        key: '43',
        zeroOffsetStoppingCharacter: '?',
        suggestedOffset: '91'
    },
    {
        key: '44',
        zeroOffsetStoppingCharacter: '!',
        suggestedOffset: '45'
    },
]
