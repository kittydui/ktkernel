#include <kt/api.h>
#include <kt/intrin.h>
#include <kt/module.h>
#include <kt/types.h>

constexpr auto CMOS_ADDRESS = 0x70;
constexpr auto CMOS_DATA = 0x71;

bool UpdateInProgress()
{
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

uint8_t GetCmosData(int reg)
{
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

uint8_t BcdToBinary(uint8_t bcd)
{
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

void CmosShutdown(KtModule* module)
{
}

KtStatus CmosRead(KtModule* module, void* buffer, size_t size)
{
    if (size < sizeof(KtDateTime))
        return KtStatus::FAILED;

    auto* time = reinterpret_cast<KtDateTime*>(buffer);

    while (UpdateInProgress())
        ;

    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;

    time->m_Second = GetCmosData(0x00);
    time->m_Minute = GetCmosData(0x02);
    time->m_Hour = GetCmosData(0x04);
    time->m_Day = GetCmosData(0x07);
    time->m_Month = GetCmosData(0x08);
    time->m_Year = GetCmosData(0x09);

    do {
        second = time->m_Second;
        minute = time->m_Minute;
        hour = time->m_Hour;
        day = time->m_Day;
        month = time->m_Month;
        year = time->m_Year;

        while (UpdateInProgress())
            ;

        time->m_Second = GetCmosData(0x00);
        time->m_Minute = GetCmosData(0x02);
        time->m_Hour = GetCmosData(0x04);
        time->m_Day = GetCmosData(0x07);
        time->m_Month = GetCmosData(0x08);
        time->m_Year = GetCmosData(0x09);
    } while ((second != time->m_Second) || (minute != time->m_Minute) || (hour != time->m_Hour) ||
             (day != time->m_Day) || (month != time->m_Month) || (year != time->m_Year));

    uint8_t status_b = GetCmosData(0x0B);

    if (!(status_b & 0x04)) {
        time->m_Second = BcdToBinary(time->m_Second);
        time->m_Minute = BcdToBinary(time->m_Minute);
        time->m_Hour = BcdToBinary(time->m_Hour);
        time->m_Day = BcdToBinary(time->m_Day);
        time->m_Month = BcdToBinary(time->m_Month);
        time->m_Year = BcdToBinary(time->m_Year);
    }

    if (!(status_b & 0x02) && (time->m_Hour & 0x80))
        time->m_Hour = ((time->m_Hour & 0x7F) + 12) % 24;

    return KtStatus::SUCCESS;
}

KtStatus ModuleEntry(KtModule* module)
{
    module->m_dispatchFunctions.Shutdown = CmosShutdown;
    module->m_dispatchFunctions.Read = CmosRead;

    return KtStatus::SUCCESS;
}

KtDeclareModule("cmos_driver", ModuleEntry);
