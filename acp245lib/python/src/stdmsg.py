STANDARD_MESSAGES = (
        dict(
            section='13.3.1',
            title='Activation attempt with no version element',
            messages = (
                dict(
                    section='.1',
                    hex='01011116000B42000D80208A12345678901234567890'
                ),
                dict(
                    section='.2',
                    hex='0103101C04000001020B422001000D80208A12345678901234567890'
                )
            )
        ),
        dict(
            section='13.3.2',
            title='Consult request: no Version Element',
            messages = (
                dict(
                    section='.1',
                    hex='01011116000B02000D80208A12345678901234567890'
                ),
                dict(
                    section='.2',
                    hex='01031018000B022001000D80208A12345678901234567890'
                )
            )
        ),
        dict(
            section='13.3.3',
            title='Deactivation attempt (invalid): no Version Element,"Tracking" deactivation',
            messages = (
                dict(
                    section='.1',
                    hex='01011116000A82000D80208A12345678901234567890'
                ),
                dict(
                    section='.2',
                    hex='01031018000A02A001130D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.1',
            title='Updating Tracking Timer without version element',
            messages=(
                dict(
                    section='.1',
                    hex='0208111D0000C2000D80208A123456789012345678900005001102003C',
                ),
                dict(
                    section='.2',
                    #    0209102204YYZZ010200C2X007001102003C01000D80208A12345678901234567890
                    hex='02091022040000010200C28007001102003C01000D80208A12345678901234567890'
                )
            )
        ),
        dict(
            section='13.4.2',
            title='Updating Multiple Parameters without version element',
            messages=(
                dict(
                    section='.1',
                    hex='020811270000C2000D80208A12345678901234567890000F001102003C001902001E0051020096'
                ),
                dict(
                    section='.2',
                    #    0209103004YYZZ010200C2X015001102003C0100001902001E0100005102009601000D80208A12345678901234567890
                    hex='02091030040000010200C28015001102003C0100001902001E0100005102009601000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.3',
            title='Consulting Single Parameter: no version element',
            messages=(
                dict(
                    section='.1',
                    hex='0208111B000002000D80208A123456789012345678900003001100'
                ),
                dict(
                    section='.2',
                    #    0209102204YYZZ010200020007001102003C01000D80208A12345678901234567890
                    hex='02091022040000010200020007001102003C01000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.4',
            title='Consulting Multiple Parameters: no version element',
            messages=(
                dict(
                    section='.1',
                    hex='02081121000002000D80208A123456789012345678900009001100001900005100'
                ),
                dict(
                    section='.2',
                    #    0209103004YYZZ01020002X015001102003C0100001902001E0100005102009601000D80208A12345678901234567890
                    hex='02091030040000010200028015001102003C0100001902001E0100005102009601000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.4',
            title='Update Vehicle Location Data: no version element',
            messages=(
                dict(
                    section='.3',
                    hex='0208111C0000C2000D80208A12345678901234567890000400470101'
                ),
                dict(
                    section='.4',
                    #    0209102104YYZZ010200C2X0060047010101000D80208A12345678901234567890
                    hex='02091021040000010200C280060047010101000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.5',
            title='Configuration TCU Service Activation/ Deactivation Message ACP 245 (Activation)',
            messages=(
                dict(
                    section='.1',
                    #    020A113A0F4674696D2E62724374696D4374696D0DBD4F22F015B3BD4F22F115BE00401680308A1234567890123456789008XXXXXXXXXXXXXXXX
                    hex='020A113A0F4674696D2E62724374696D4374696D0DBD4F22F015B3BD4F22F115BE00401680308A12345678901234567890080102030405060708'
                ),
                dict(
                    section='.2',
                    #    0203101D04YYZZ0102000042X001000D80208A12345678901234567890
                    hex='0203101D04000001020000428001000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.4.5',
            title='Configuration TCU Service Activation / Deactivation Message ACP 245 (Deactivation)',
            messages=(
                dict(
                    section='.3',
                    hex='020A11150000000D80208A12345678901234567890'
                ),
                dict(
                    section='.4',
                    #    0203101D04YYZZ0102000082X001000D80208A12345678901234567890
                    hex='0203101D04000001020000828001000D80208A12345678901234567890'
                )
            )
        ),
        dict(
            section='13.5.1',
            title='Blocking request: no version element (shortest message)',
            messages=(
                dict(
                    section='.1',
                    hex='0602111700018001020D80208A12345678901234567890'
                ),
                dict(
                    section='.2',
                    #    0603101D04YYZZ01020180010201000D80208A12345678901234567890
                    hex='0603101D04000001020180010201000D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.5.2',
            title='Unblocking request: no version element (shortest message)',
            messages=(
                dict(
                    section='.1',
                    hex='0602111700018001030D80208A12345678901234567890'
                ),
                dict(
                    section='.2',
                    #    0603101D04YYZZ01020180010301000D80208A12345678901234567890
                    hex='0603101D04000001020180010301000D80208A12345678901234567890'
                )
            )
        ),
        dict(
            section='13.6.1.1',
            title='Vehicle Position Message with Vehicle Location Data (Data type 0x0047 = True)',
            messages=(
                dict(
                    section='',
                    #    0A02103A04YYZZ01014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A1234567890123456789008808000010380C01100
                    hex='0A02103A04000001014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A1234567890123456789008808000010380C01100'
                ),
            )
        ),
        dict(
            section='13.6.1.2',
            title='Vehicle Position Message without Vehicle Location Data (Data type 0x0047 = False)',
            messages=(
                dict(
                    section='',
                    #    0A02102604YYZZ01014D6D22F8000D80208A1234567890123456789008808000010380C01100
                    hex='0A02102604000001014D6D22F8000D80208A1234567890123456789008808000010380C01100'
                ),
            )
        ),
        dict(
            section='13.7.1.1',
            title='Theft Alarm Keep Alive',
            messages=(
                dict(
                    section='',
                    hex='0B0410120D80208A12345678901234567890'
                ),
            )
        ),
        dict(
            section='13.7.1.2',
            shortname='Theft Alarm Notif - Ignition On Notif - With Location',
            title='Notification of "Ignition ON" with Vehicle Location Data (Data type 0x0047 = True)',
            messages=(
                dict(
                    section='',
                    # 0B01103A04YYZZ01014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A123456789012345678900880C000010380C01100
                    hex='0B01103A04000001014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A123456789012345678900880C000010380C01100'
                ),
            )
        ),
        dict(
            section='13.7.1.3',
            shortname='Theft Alarm Notif - Ignition On Notif - No  Location',
            title='Notification of "Ignition ON" without Vehicle Location Data (Data type 0x0047 = False)',
            messages=(
                dict(
                    section='',
                    #    0B01102604YYZZ01014D6D22F8000D80208A123456789012345678900880C000010380C01100
                    hex='0B01102604000001014D6D22F8000D80208A123456789012345678900880C000010380C01100'
                ),
            )
        ),
        dict(
            section='13.7.1.4',
            shortname='Theft Alarm Pos - With Location',
            title='Theft Alarm in event mode Vehicle Position Message with Vehicle Location Data (Data type 0x0047 = True)',
            messages=(
                dict(
                    section='',
                    #    0B03103A04YYZZ01014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A1234567890123456789008808000010380C01100
                    hex='0B03103A04000001014D6D22F8141311300000F6F2BED0FAF5BA2802E4096F0678800D80208A1234567890123456789008808000010380C01100'
                ),
            )
        ),
        dict(
            section='13.7.1.5',
            shortname='Theft Alarm Pos - No Location',
            title='Theft Alarm in event mode Vehicle Position Message without Vehicle Location Data (Data type 0x0047 = False)',
            messages=(
                dict(
                    section='',
                    #    0B03102604YYZZ01014D6D22F8000D80208A1234567890123456789008808000010380C01100
                    hex='0B03102604000001014D6D22F8000D80208A1234567890123456789008808000010380C01100'
                ),
            )
        ),
)

from acp245 import pdu

STANDARD_BY_SECTION_HEX = dict(
    [
        (
            '%s%s' % (x['section'], y['section']),
            y['hex']
        )
        for x in STANDARD_MESSAGES for y in x['messages']
    ]
)

STANDARD_BY_SECTION = dict(
    [
        (
            '%s%s' % (x['section'], y['section']),
            pdu.msg_read(hex=y['hex'])[0]
        )
        for x in STANDARD_MESSAGES for y in x['messages']
    ]
)
