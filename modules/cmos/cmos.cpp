#include <kt/api.h>
#include <kt/intrin.h>
#include <kt/module.h>
#include <kt/types.h>

constexpr auto cmos_address = 0x70;
constexpr auto cmos_data = 0x71;

static bool update_in_progress()
{
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}

static uint8_t get_cmos_data(int reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}

static uint8_t bcd_to_binary(uint8_t bcd)
{
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

static void cmos_shutdown(kt_module*)
{
}

static kt_status cmos_read(kt_module*, void* buffer, size_t size)
{
    if (size < sizeof(kt_date_time))
        return kt_status::failed;

    auto* time = reinterpret_cast<kt_date_time*>(buffer);

    while (update_in_progress())
        ;

    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;

    time->second = get_cmos_data(0x00);
    time->minute = get_cmos_data(0x02);
    time->hour = get_cmos_data(0x04);
    time->day = get_cmos_data(0x07);
    time->month = get_cmos_data(0x08);
    time->year = get_cmos_data(0x09);

    do {
        second = time->second;
        minute = time->minute;
        hour = time->hour;
        day = time->day;
        month = time->month;
        year = time->year;

        while (update_in_progress())
            ;

        time->second = get_cmos_data(0x00);
        time->minute = get_cmos_data(0x02);
        time->hour = get_cmos_data(0x04);
        time->day = get_cmos_data(0x07);
        time->month = get_cmos_data(0x08);
        time->year = get_cmos_data(0x09);
    } while ((second != time->second) || (minute != time->minute) || (hour != time->hour) ||
             (day != time->day) || (month != time->month) || (year != time->year));

    uint8_t status_b = get_cmos_data(0x0B);

    if (!(status_b & 0x04)) {
        time->second = bcd_to_binary(time->second);
        time->minute = bcd_to_binary(time->minute);
        time->hour = bcd_to_binary(time->hour);
        time->day = bcd_to_binary(time->day);
        time->month = bcd_to_binary(time->month);
        time->year = bcd_to_binary(time->year);
    }

    if (!(status_b & 0x02) && (time->hour & 0x80))
        time->hour = ((time->hour & 0x7F) + 12) % 24;

    return kt_status::success;
}

static kt_status module_entry(kt_module* module)
{
    module->dispatch_functions.shutdown = cmos_shutdown;
    module->dispatch_functions.read = cmos_read;

    return kt_status::success;
}

kt_declare_module("cmos_driver", module_entry);
