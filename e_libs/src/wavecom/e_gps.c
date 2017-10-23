/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_gps.h"
#include "e_gps_wm.h"

#include "adl_global.h"
#include "eRide.h"
#include "eRideNmea.h"

#include "e_port.h"
#include "e_log.h"
#include "e_errors.h"
#include "e_gps_flash.h"
#include "e_time.h"

#define OPUS_INPUT_BUFFER_SIZE  4096
#define OPUS_OUTPUT_BUFFER_SIZE 4096
#define OPUS_BAUD_RATE          57600
#define OPUS_PASSTHRU_STRING    "<MODE=PT>"
#define OPUS_PASSTHRU_NMEA      "$PERDSYS,EXIT,4"

/* Constant defining the maximum data that can be sent to the GPS chipset */
#define OPUS_FCM_TX_MAX  ((u8) 250)

/* Number of GPIOs to be used for CGPS board */
#define GPS_GPIO_COUNT  (3)

/* Constant which defines the Low value (OFF state) of LED */
#define LED_GPS_OFF     (0)

/* Constant which defines the High value (ON State) of LED */
#define LED_GPS_ON      (1)

/* GPIO used for NRST pin connection */
static adl_ioDefs_t ADL_IO_NRST = 0;

/* GPIO used for GPS enable pin */
static adl_ioDefs_t ADL_IO_GPS = 0;

/* GPIO used for Temperature Compensated Crystal Oscillator */
static adl_ioDefs_t ADL_IO_TCXO = 0;

/* GPIO Handle for GPS chipset */
static s32 gpio_handle = -1;

/* Non-volatile storage buffer for satellite data */
static erNvDataStruct nvData;
static U8 nvDataBuff[ER_NVDATA_SIZE];

static adl_tmr_t *check_gps_data_timer = NULL;
static adl_tmr_t *do_gps_start_timer = NULL;
static adl_tmr_t *send_buffer_retry_timer = NULL;
static adl_tmr_t *gpio_init_timer = NULL;

static u8 rx_buff[OPUS_INPUT_BUFFER_SIZE];
static u16 rx_buff_lim = 0;

static bool no_tx_credits = false;
static u8 tx_buff[OPUS_OUTPUT_BUFFER_SIZE];
static u16 tx_buff_lim = 0;

static adl_port_e gps_uart = 0;
static s8 fcm_handle = -1;

static e_gps_pvt_handler gps_pvt_handler = NULL;
static e_gps_err_handler gps_err_handler = NULL;
static e_gps_stop_handler gps_stop_handler = NULL;

static e_gps_position curr_pos;
static bool _wrote_new_nvdata = FALSE;

typedef enum {
    E_GPS_INIT,
    E_GPS_ACTIVE,
    E_GPS_INACTIVE,
    E_GPS_STOPPING
} gps_state;

static gps_state state = E_GPS_INIT;

static int opus_baud_rate=0;

#ifdef WM_FASTRACK
/*  Temperature Control Oscillator(TCO) Circuit constants.
 *    NOTE:  These values are based on a 0.5 ppm Rakon TXCO crystal
 */
static erTcoConstantsStruct tcoConstOPUS1 = {
    (U32) (300 << 12),      // max aging
    (U32) (180 << 12),      // hysteresis
    (S32) (0x7fffffff),     // temp reference
    (S32) (80  << 12),      // max temp full
    (S32) (-30  << 12),     // min temp full
    (U32) (150 << 12),      // freq stability full
    (U32) (30 << 12),       // freq slope full
    (S32) (50 << 12),       // max temp reduced
    (S32) (-10 << 12),      // min temp reduced
    (U32) (90 << 12),       // freq stability reduced
    (U32) (15 << 12),       // freq slope reduced
    (U16)  21               // TCO table entries below
};
#elif defined(WM_MOBIPOWER)
static const erTcoConstantsStruct tcoConstOPUS3 = {
    (U32) (300 << 12),  // max aging
    (U32) (150 << 12),  // hysteresis
    (S32) (0x7fffffff), // temp reference
    (S32) (95  << 12),  // max temp full
    (S32) (-95 << 12),  // min temp full
    (U32) (150 << 12),  // freq stability full
    (U32) (30 << 12),   // freq slope full
    (S32) (95  << 12),  // max temp reduced
    (S32) (-95 << 12),  // min temp reduced
    (U32) (150 << 12),  // freq stability reduced
    (U32) (30 << 12),   // freq slope reduced

    (U16)  29           // TCO table entries below
};
#endif

/*
 *  TCO count to temperature  conversion table.
 */
#ifdef WM_FASTRACK
static erTcoTableStruct tcoTableOPUS1[] = {
    /*{TCO,Temp,} */
    {29,-20}, {36,-15}, {42,-10}, {54,-5}, {68,0}, {85,5}, {106,10}, {137,15},
    {154,20}, {193,25}, {224,30}, {266,35}, {311,40}, {356,45}, {418,50},
    {472,55}, {523,60}, {593,65}, {673,70}, {740,75}, {780,78}
};
#elif defined(WM_MOBIPOWER)
static const erTcoTableStruct tcoTableOPUS3[] = {
    /* {TCO,Temp} */
    {   1,-95}, {  32,-35}, {  43,-30}, {  56,-25}, {  74,-20}, {  95,-15}, 
    { 121,-10}, { 153, -5}, { 192,  0}, { 239,  5}, { 295, 10}, { 361, 15}, 
    { 439, 20}, { 531, 25}, { 637, 30}, { 760, 35}, { 902, 40}, {1065, 45}, 
    {1251, 50}, {1462, 55}, {1700, 60}, {1969, 65}, {2270, 70}, {2606, 75}, 
    {2981, 80}, {3397, 85}, {3857, 90}, {4364, 95}, {4922,100} 
};
#endif

#if defined(WM_MOBIPOWER)
static u8 gpio_init_state = 0;
#endif

static void _set_core_to_passthru(void);
static void _set_core_to_passthru_timer(u8 id, void *context);
static int _configure_gps_port(void);
static void _configure_gps_port_timer(u8 id, void *context);

static void _switch_state(gps_state new_state)
{
    state = new_state;
}

static double er_u2f(U32 v, S32 e) {
    double r;
    if (e > 0) {
        return (double) (v * (2 << e));
    } else {
        r = (1.0/2.0);
        while(++e) {
            r = r * (1.0/2.0);
        };
        return r*v;
    }
}

static double er_i2f(S32 v, S32 e) {
    double r;
    if (e > 0) {
        return (double) (v * (2 << e));
    } else {
        r = (1.0/2.0);
        while(++e) {
            r = r * (1.0/2.0);
        };
        return r*v;
    }
}

static double to_degrees(double radians) 
{
    return radians * (180.0/3.141592654);
}

static void rx_buff_clear(void)
{
    rx_buff_lim = 0;
}

static void tx_buff_clear(void)
{
    tx_buff_lim = 0;
}

/*
 * er Callbacks
 */
static U16 cb_put_debug(U8 *buf, U16 length)
{
    /* TODO process debug data */
	return length;
}

static long cb_get_msec_timer(void)
{
    struct timeval now;
    if(gettimeofday(&now, NULL)) {
        E_FATAL("gettimeofday failed");
        return 0;
    }
    return (long) (now.tv_sec * 1000 + now.tv_usec/1000);
}

static void cb_sleep (U16 msec)
{
	unsigned long diff = 0;
	long initial_time;

	initial_time = cb_get_msec_timer();

    E_TRACE("Sleeping for %lums", (unsigned long) msec);

	while (diff < (unsigned long) msec) {
		diff = (unsigned long) (cb_get_msec_timer() - initial_time);
	}
}

static U8 cb_put_gps_data(U8 *buff, U16 length)
{
    u8 written;
    u8 to_write;
	s8 status;

    e_assert(buff != NULL);

    if (0 == length) {
        return 0;
    }

    to_write = (length > 256) ? (u8) 256 : (u8) length;

    if (no_tx_credits || (tx_buff_lim > 0) || (fcm_handle < 0)) {
        /* store in buffer */
        if ((tx_buff_lim + to_write) < OPUS_OUTPUT_BUFFER_SIZE) {
            E_TRACE("gps buff data");
            (void) e_mem_cpy(&tx_buff[tx_buff_lim], buff, to_write);
            tx_buff_lim += to_write;
            written = to_write;
        } else {
            if (fcm_handle < 0) {
                E_WARN("gps tx buffer overflow and no FCM handle, ignoring data");
                written = to_write;
            } else {
                E_WARN("gps tx buffer overflow.");
                written = 0;
            }
        }
    } else {
        /* send directly */
        if (fcm_handle >= 0) {
            status = adl_fcmSendData(fcm_handle, buff, to_write);
            if (OK == status) {
                E_TRACE("gps, wrote");
                written = to_write;
            } else if (ADL_FCM_RET_OK_WAIT_RESUME == status) {
                E_TRACE("gps, no credits");
                no_tx_credits = true;
                written = to_write;
            } else {
                E_FATAL("err: %lu %x", to_write, fcm_handle);
                ERR_LOG(adl_fcmSendData, status);
                written = 0;
            }
        }
    }
	return (U8) written;
}

/*
 * Timer handlers 
 */
static void notify_err(long err)
{
    if (gps_err_handler != NULL) {
        /* FIXME may lose some precision...*/
        gps_err_handler((s32) err);
    } else {
        E_DBG("no error handler, ignoring gps error notification");
    }
}

static s16 save_nv_data(void)
{
    E_DBG("Saving NV data");
    return e_gps_write_flash(nvData.nvData, nvData.length);
}

static void notify_gps_data(EIT gps_data_mask)
{
    static bool fix = false;

    E_TRACE("notify_gps_data: 0x%x", gps_data_mask);

    if (gps_data_mask & ER_PVT_AVAIL) {
        static ascii nmea[200];
        s16 len;
	    erPvtStruct	*erPvt = NULL;

        erNmeaSetHoldState(gps_data_mask);

	    erPvt = erGetPvt();
        curr_pos.valid = erPvt->pvtFixSource;
        curr_pos.lat = to_degrees(er_i2f(erPvt->pvtLlaFilt[0],-28));
    	curr_pos.lon = to_degrees(er_i2f(erPvt->pvtLlaFilt[1],-28));
	    curr_pos.altitude = er_i2f(erPvt->pvtLlaFilt[2], -6);
        curr_pos.heading = erPvt->pvtHeadingFilt;
        curr_pos.speed = (u8) er_u2f(erPvt->pvtSpeed, -12);
        curr_pos.pdop = (u8) er_u2f(erPvt->pvtPdop, -2);
        curr_pos.vdop = (u8) er_u2f(erPvt->pvtVdop, -2);
        curr_pos.hdop = (u8) er_u2f(erPvt->pvtHdop, -2);
        curr_pos.tdop = 0;

        (void) e_mem_set(nmea, 0, sizeof(nmea));
        len = erNmeaGetRMC((unsigned char *)nmea);
        if ((len < sizeof(nmea)) && (len >= 0)) {
            nmea[len] = '\0';
            E_DBG("NMEA: '%s'", nmea);
        }
        fix = erPvt->pvtFixSource;
    }

    if (gps_data_mask & ER_SVSTAT_AVAIL) {
        erSvStatusStruct *erSvStatus = NULL;
        int i;

        erSvStatus = erGetSvStatus();
        for (i = 0; i <
            (E_GPS_MAX_SAT < ER_SV_STATUS_NUM_SVS ?
             E_GPS_MAX_SAT : ER_SV_STATUS_NUM_SVS); i++) {
            curr_pos.sats[i] = erSvStatus->svID[i];
        }
    }

    if (fix) {
        if ((gps_data_mask & ER_NVDATA_AVAIL) && !_wrote_new_nvdata) {
            _wrote_new_nvdata = TRUE;
            if(save_nv_data()) {
                E_ERR("Error saving GPS data");
            }
        }
        if (gps_pvt_handler != NULL) {
            gps_pvt_handler();
        }
    }
}

static void check_gps_data(u8 timerId, void *context)
{
    EIT gps_data_mask = 0;
    if (E_GPS_STOPPING == state || E_GPS_INIT == state) {
        return;
    }
    if (erGetGpsState() == ER_GPS_STATE_ON || rx_buff_lim > 0) {
        E_TRACE("checking for GPS data, buff_lim %ld",
                (unsigned long) rx_buff_lim);
        if (rx_buff_lim > 0) {
            gps_data_mask = erGpsCoreTask(
                    (unsigned char*)rx_buff,
                    (unsigned short int) rx_buff_lim);
            rx_buff_clear();
            if (gps_data_mask != 0) {
                notify_gps_data(gps_data_mask);
            }
	    }
        _switch_state(E_GPS_ACTIVE);
    } else if(E_GPS_ACTIVE == state) {
        E_WARN("GPS closed while checking GPS data availability.");
        _switch_state(E_GPS_INACTIVE);
        notify_err(GPS_ERR_GPS_STOPPED);
    } else {
        E_DBG("Waiting for GPS init.");
    }
}

static void do_gps_start(u8 timer_id, void *context) 
{
    int res;

    E_STACK("do_gps_start enter");

    if (state != E_GPS_INIT) {
        E_DBG("GPS not initing, skipping start.");
        goto exit;
    }

    do_gps_start_timer = 0;


    E_DBG("GPS starting.");

    erNmeaInit(FALSE);
    erNmeaPerRunInit(FALSE);
	res = erGpsStart();
    if (res != 0) {
        if (res == -1) {
            E_INFO("GPS already running");
        } else {
            E_ERR("Unable to initialize GPS core");
            notify_err(GPS_ERR_GPS_START);
        }
        return;
    }

    E_DBG("GPS subscribing timer for GPS check data.");

    _switch_state(E_GPS_INACTIVE);
	check_gps_data_timer = adl_tmrSubscribe(TRUE, 11, ADL_TMR_TYPE_TICK, 
            check_gps_data);
    if (!(check_gps_data_timer > 0)) {
        ERR_LOG(adl_tmrSubscribe, check_gps_data_timer);
        notify_err(GPS_ERR_GPS_CHECK);
    }

exit:
    E_STACK("do_gps_start exit");
}

static int gps_init(void)
{
    int start_type;
    int rc = -1;
    int res;
    erCallbackStruct er_callback;

    start_type = ER_START_TYPE_HOT;

    e_gps_flash_init();

    (void) e_mem_set(&er_callback, 0, sizeof(er_callback));

    /* Register the call back functions, must be the first config action */
    er_callback.cbPutGps   = cb_put_gps_data;
    er_callback.cbPutDebug = cb_put_debug;
    er_callback.cbGetMsecTimer = cb_get_msec_timer;
    er_callback.cbSleep = cb_sleep;
    E_TRACE("erRegisterCallbacks calling");
    erRegisterCallbacks(&er_callback);

    /* FIXME: store/load satellite data in NV memory */
    /* FIXME: Make sure GPS is stoped since we are going to overwrite a
     * shared buffer... */
    (void) e_mem_set(&nvData, 0, sizeof(nvData));
    (void) e_mem_set(nvDataBuff, 0, sizeof(nvDataBuff));

    nvData.length = ER_NVDATA_SIZE;
    nvData.nvData = nvDataBuff;
    /* Used if nvData.nvData is invalid */
    nvData.nvDefaults = (U8*) e_gps_flash_default();

    /* deprecated on 3.04.2020...*/
    nvData.isNew = NVDATA_DEFAULT_USED;
    if (start_type != ER_START_TYPE_FACTORY_COLD) {
        if ((e_gps_read_flash(nvData.nvData, nvData.length) != nvData.length)
                || (*nvData.nvData == 0xFF)) {
            E_WARN("Performing GPS Factory Cold Start, almanac read from flash failed");
            start_type = ER_START_TYPE_FACTORY_COLD;
        } else {
            E_DBG("Using GPS almanac data from flash");
            nvData.isNew = NVDATA_LATEST_USED;
        }
    } else {
        E_DBG("Performing GPS Factory Cold Start");
    }

    erSetNvData(&nvData);

    res = erSetStartType(start_type);
    if (res != 1) {
        E_ERR("GPS unsuccessfully configured: %d", res);
        notify_err(GPS_ERR_CONF);
        goto exit;
    }

    E_DBG("GPS Start type set to %x", start_type);

    /****  Configure the eRide GPS core library ****/

    E_TRACE("erSetTcoData calling");
#ifdef WM_FASTRACK
    erSetTcoData(&tcoConstOPUS1, tcoTableOPUS1);
#elif defined(WM_MOBIPOWER)
    erSetTcoData(
            (erTcoConstantsStruct *)&tcoConstOPUS3, 
            (erTcoTableStruct *)tcoTableOPUS3);
#endif

    /* Enable position propagation for two seconds */
    //E_TRACE("erSetLatencyPositionPropagation calling");
    //erSetPositionOutagePropagation(2);
    //erSetLatencyPositionPropagation(2);

    E_TRACE("erSetMxMode calling");
    //erSetMxMode(ER_MX_MODE_WRONLY); // not included in AppQuery sample

    E_DBG("GPS data restored from NV storage");

    rc = 0;

exit:
    return rc;

}

static void gpio_init(u8 timer_id, void *context) 
{
    s32 res = -1;
    adl_ioDefs_t gpio_config[GPS_GPIO_COUNT];

#ifdef WM_FASTRACK
    E_STACK("gpio_init enter");

    gpio_init_timer = NULL;

    (void) e_mem_set(gpio_config, 0, sizeof(gpio_config));
    // Assign GPIOs according to the Wireless CPU
    ADL_IO_NRST = E_GPS_GPIO_NRST;
    ADL_IO_GPS  = E_GPS_GPIO_GPS;
    ADL_IO_TCXO = E_GPS_GPIO_TCXO;

    gpio_config[0] = ADL_IO_NRST | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    gpio_config[1] = ADL_IO_GPS  | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    gpio_config[2] = ADL_IO_TCXO | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;

	gpio_handle  = adl_ioSubscribe(GPS_GPIO_COUNT, gpio_config, 0, 0, 0);
    if (gpio_handle < 0) {
        ERR_LOG(adl_ioSubscribe, gpio_handle);
        notify_err(GPS_ERR_GPIO_CONF);
        return;
    }

    res = adl_ioWriteSingle(gpio_handle, &ADL_IO_GPS , TRUE);
    if (res != OK) {
        ERR_LOG(adl_ioWriteSingle, res);
        notify_err(GPS_ERR_GPIO_CONF);
        return;
    }

	res = adl_ioWriteSingle(gpio_handle, &ADL_IO_NRST, TRUE);
    if (res != OK) {
        ERR_LOG(adl_ioWriteSingle, res);
        notify_err(GPS_ERR_GPIO_CONF);
        return;
    }

	do_gps_start_timer = adl_tmrSubscribe(FALSE, 10, ADL_TMR_TYPE_100MS, 
            do_gps_start);
    if(!(do_gps_start_timer > 0)) {
        ERR_LOG(adl_tmrSubscribe, do_gps_start_timer);
        notify_err(GPS_ERR_CONF);
        return;
    }

#elif defined(WM_MOBIPOWER)
    adl_tmr_t *tmr;

    E_STACK("gpio_init enter");

    gpio_init_timer = NULL;

    E_DBG("gpio_init state: %lld", (long long) gpio_init_state);

    (void) e_mem_set(gpio_config, 0, sizeof(gpio_config));
    // Assign GPIOs according to the Wireless CPU
    ADL_IO_NRST = E_GPS_GPIO_NRST;
    ADL_IO_GPS  = E_GPS_GPIO_GPS;
    ADL_IO_TCXO = E_GPS_GPIO_TCXO;

    gpio_config[0] = ADL_IO_NRST | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    gpio_config[1] = ADL_IO_GPS  | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    gpio_config[2] = ADL_IO_TCXO | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;

    switch(gpio_init_state)
    {
        case 0:
	        gpio_handle  = adl_ioSubscribe(GPS_GPIO_COUNT, gpio_config,
                    0, 0, 0);
            if (gpio_handle < 0) {
                ERR_LOG(adl_ioSubscribe, gpio_handle);
                notify_err(GPS_ERR_GPIO_CONF);
                return;
            }
            gpio_init_state++;
            tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS, gpio_init);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            break;

        case 1:
	        res = adl_ioWriteSingle(gpio_handle, &ADL_IO_NRST, FALSE);
            if (res != OK) {
                ERR_LOG(adl_ioWriteSingle, res);
                notify_err(GPS_ERR_GPIO_CONF);
                return;
            }
            gpio_init_state++;
            tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS, gpio_init);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            break;

        case 2:
            res = adl_ioWriteSingle(gpio_handle, &ADL_IO_GPS , TRUE);
            if (res != OK) {
                ERR_LOG(adl_ioWriteSingle, res);
                notify_err(GPS_ERR_GPIO_CONF);
                return;
            }
            gpio_init_state++;
            tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS, gpio_init);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            break;

        case 3:
	        res = adl_ioWriteSingle(gpio_handle, &ADL_IO_NRST, TRUE);
            if (res != OK) {
                ERR_LOG(adl_ioWriteSingle, res);
                notify_err(GPS_ERR_GPIO_CONF);
                return;
            }
            gpio_init_state++;
            tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS, gpio_init);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            break;

        case 4:
            res = adl_ioWriteSingle(gpio_handle, &ADL_IO_TCXO, TRUE);
            if (res != OK) {
                ERR_LOG(adl_ioWriteSingle, res);
                notify_err(GPS_ERR_GPIO_CONF);
                return;
            }
            gpio_init_state++;
            tmr = adl_tmrSubscribe(FALSE, 1, ADL_TMR_TYPE_100MS, gpio_init);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            break;

        case 5:
            tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS,
                    _set_core_to_passthru_timer);
            if ((tmr == NULL) || (tmr < 0)) {
                ERR_LOG(adl_tmrSubscribe, tmr);
            }
            gpio_init_state++;
            break;
        case 6:
            if (opus_baud_rate == OPUS_BAUD_RATE) {
                E_DBG("GPS port correctly configured");
                if ((erGetGpsState() != ER_GPS_STATE_ON)) {
                    gps_init();

    	            do_gps_start_timer = adl_tmrSubscribe(FALSE, 10,
                            ADL_TMR_TYPE_100MS, do_gps_start);
                    if(!(do_gps_start_timer > 0)) {
                        ERR_LOG(adl_tmrSubscribe, do_gps_start_timer);
                        notify_err(GPS_ERR_CONF);
                        goto exit;
                    }
                }
                gpio_init_state++;
            } else {
                tmr = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS,
                        _set_core_to_passthru_timer);
                if ((tmr == NULL) || (tmr < 0)) {
                    ERR_LOG(adl_tmrSubscribe, tmr);
                }
            }
            break;

        case 7:
            /* already inited */
            break;
        default:
            E_FATAL(" Illegal gpio_init state: %d", gpio_init_state) ;
            break;
    }

#endif
exit:
    E_STACK("gpio_init exit");
}

static void send_buffer(void);

static void send_buffer_retry(u8 id, void * context)
{
    send_buffer_retry_timer = NULL;
    send_buffer();
}

static void schedule_send_buffer_retry(void)
{
    send_buffer_retry_timer = adl_tmrSubscribe (FALSE, 3, ADL_TMR_TYPE_TICK,
            send_buffer_retry);
    if (!(send_buffer_retry_timer > 0)) {
        ERR_LOG(adl_tmrSubscribe, send_buffer_retry_timer);
    }
}

static void send_buffer(void)
{
    s8 status;
    u16 sent;


    if (fcm_handle < 0) {
        E_FATAL("invalid fcm_handle");
        return;
    }

    sent = tx_buff_lim > OPUS_FCM_TX_MAX ? OPUS_FCM_TX_MAX : tx_buff_lim;

    status = adl_fcmSendData(fcm_handle, tx_buff, sent);
    if (status != OK) {
        if (status == ADL_FCM_RET_OK_WAIT_RESUME) {
            no_tx_credits = true;
        } else {
            ERR_LOG(adl_fcmSendData, status);
            schedule_send_buffer_retry();
            return;
        }
    }

    if (tx_buff_lim > sent) {
        (void) e_mem_cpy(tx_buff, &tx_buff[sent], tx_buff_lim - sent);
        tx_buff_lim -= sent;
        schedule_send_buffer_retry();

    } else {
        e_assert(sent == tx_buff_lim);
        tx_buff_clear();
    }
}

static bool ctrl_handler(u8 event)
{
    s8 res;
    s32 res2;

	switch(event) {
		case ADL_FCM_EVENT_FLOW_OPENNED:
            E_DBG("FCM event flow opened");
            if (state == E_GPS_INIT) {
    			res = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_DATA);
                if (res != OK) {
                    ERR_LOG(adl_fcmSwitchV24State, res);
                    notify_err(GPS_ERR_FCM_DATA);
                }
            } else {
                _switch_state(E_GPS_INACTIVE);
                notify_err(GPS_ERR_GPS_STOPPED);
                E_WARN("Flow opened while not in init mode");
            }
            break;
		case ADL_FCM_EVENT_V24_DATA_MODE:
            E_DBG("Switched to FCM v24 data mode");
            if (state == E_GPS_INIT) {
    			gpio_init_timer = adl_tmrSubscribe ( FALSE, 10, ADL_TMR_TYPE_100MS,
                        gpio_init );
                if (!(gpio_init_timer > 0)) {
                    ERR_LOG(adl_tmrSubscribe, gpio_init_timer);
                    notify_err(GPS_ERR_GPIO_CONF);
                }
            } else {
                E_WARN("Switched to data mode while not in init mode");
                _switch_state(E_GPS_INACTIVE);
                notify_err(GPS_ERR_GPS_STOPPED);
            }
		    break;
		case ADL_FCM_EVENT_RESUME:
            E_DBG("FCM event resume");
            if (no_tx_credits) {
                no_tx_credits = false;
                send_buffer();
            }
            break;
		case ADL_FCM_EVENT_V24_AT_MODE:
            E_DBG("Switched to FCM v24 AT mode");
            /* Switched back to AT mode, unregister FCM handle */
            res2 = adl_fcmUnsubscribe(fcm_handle);
            if (res2 != OK) {
                ERR_LOG(adl_fcmUnsubscribe, res2);
                goto exit;
            }
            E_DBG("Unsubscribed to FCM handler.");
            fcm_handle = -1;
            rx_buff_clear();
            break;
		case ADL_FCM_EVENT_FLOW_CLOSED:
            E_DBG("FCM flow closed");
#ifdef WM_FASTRACK
            notify_err(GPS_ERR_GPS_STOPPED);
#elif defined(WM_MOBIPOWER)
            if (E_GPS_INIT == state) {
                adl_tmr_t *tmr = NULL;
                /* The GPS port was closed successfully;
                 * now reopen with new baud rate */
                tmr = adl_tmrSubscribe(FALSE, 1, ADL_TMR_TYPE_TICK,
                        _configure_gps_port_timer);
                if(!(tmr > 0)) {
                    ERR_LOG(adl_tmrSubscribe, tmr);
                    notify_err(GPS_ERR_UART_CONF);
                    goto exit;
                }

                /* Still need to keep looking for valid CGPS-PVT stream */
                tmr = adl_tmrSubscribe (FALSE, 20, ADL_TMR_TYPE_100MS,
                        _set_core_to_passthru_timer);
                if(!(tmr > 0)) {
                    ERR_LOG(adl_tmrSubscribe, tmr);
                    notify_err(GPS_ERR_UART_CONF);
                    goto exit;
                }
            } else if (E_GPS_STOPPING == state) {
                _switch_state(E_GPS_INIT);
                if (gps_stop_handler) {
                    gps_stop_handler();
                }
            } else {
                _switch_state(E_GPS_INACTIVE);
                notify_err(GPS_ERR_GPS_STOPPED);
            }
#endif
            break;
		case ADL_FCM_EVENT_V24_DATA_MODE_EXT:
            E_DBG("Switched to FCM v24 data mode ext");
            break;
		case ADL_FCM_EVENT_V24_AT_MODE_EXT:
            E_DBG("Switched to FCM v24 AT mode ext");
            break;
		case ADL_FCM_EVENT_MEM_RELEASE:
            E_TRACE("FCM mem release");
            break;
        default:
            E_ERR("Unknown FCM control event: %u", event);
	}

exit:
	return TRUE;
}

static bool data_handler(u16 length, u8 *data)
{
    if ((((u32)rx_buff_lim) + ((u32)length)) < OPUS_INPUT_BUFFER_SIZE) {
        (void) e_mem_cpy(&rx_buff[rx_buff_lim], data, length);
        rx_buff_lim += length;
    } else {
        E_ERR("Received too much data from GPS for input buffer: %u", length);
    }

    return TRUE;
}

static int _init_fcm(void) 
{
    int rc;

    /* Open the Data FCM flow on the DATAGPS */
    rx_buff_clear();
    fcm_handle = adl_fcmSubscribe(gps_uart,
            (adl_fcmCtrlHdlr_f)ctrl_handler,
            (adl_fcmDataHdlr_f)data_handler);

    if (fcm_handle < 0) {
        ERR_LOG(adl_fcmSubscribe, fcm_handle);
        notify_err(GPS_ERR_FCM);
        rc = -1;
        goto exit;
    }
    E_DBG("Subscribed to FCM handler.");

    rc = 0;
exit:
    return rc;
}

static bool uart_config_handler(adl_atResponse_t *response)
{
    E_DBG("UART config handler called");

	switch(response->RspID) {
	case ADL_STR_OK:
		_init_fcm();
	    break;
	default:
        {
        ascii *std_resp = NULL;
        std_resp = adl_strGetResponse(response->RspID);
        if (std_resp != NULL) {
            E_ERR("GPS UART configuration failed: %s", std_resp);
            adl_memRelease(std_resp);
        } else {
            E_ERR("GPS UART configuration failed: unknown AT response.");
        }
        notify_err(GPS_ERR_UART_CONF);
        }
    	break;
	}
    /* do not send response thru port */
    return false;
}

static void _configure_gps_port_timer(u8 id, void *context)
{
    if(_configure_gps_port() != 0) {
        notify_err(GPS_ERR_UART_CONF);
    }
}

static int _configure_gps_port(void)
{
    ascii atcommand[80];
    s8 res = 0;

    (void) e_mem_set(atcommand, 0, sizeof(atcommand));

    /*@-bufferoverflowhigh@*/
#ifdef WM_FASTRACK
    wm_sprintf(atcommand, "AT+WIND=0;+IPR=%d;+ICF=2,0;+IFC=0,0",
            opus_baud_rate);
#elif defined(WM_MOBIPOWER)
    if (OPUS_BAUD_RATE == opus_baud_rate) {
        /* set to 8 data bits, 1 stop bit, parity odd */
        wm_sprintf(atcommand, "AT+WIND=0;+IPR=%d;+ICF=2,0;+IFC=0,0",
                opus_baud_rate);
    } else {
        /* set to 8 data bits, 1 stop bit, parity none */
        wm_sprintf(atcommand, "AT+WIND=0;+IPR=%d;+ICF=3,4;+IFC=0,0", 
                opus_baud_rate);
    }
#endif
    /*@+bufferoverflowhigh@*/

    res = adl_atCmdCreate(atcommand, 
            ADL_AT_PORT_TYPE(gps_uart, FALSE), 
            uart_config_handler, "*", NULL);

    if (res != OK) {
        ERR_LOG(adl_atCmdCreate, res);
        return -1;
    } else {
        return 0;
    }
}

static bool _uart_open_response_handler(adl_atResponse_t *paras)
{
    _configure_gps_port();
}

static void _set_core_to_passthru_timer(u8 id, void *context)
{
    if (fcm_handle >= 0) {
        _set_core_to_passthru();
    } else {
        E_DBG("FCM closed before timer executed");
    }
}

static void _set_core_to_passthru(void) {
    static int loop_cnt = 0;
    adl_tmr_t *passthru_timer;
    s8 res;

    e_assert(fcm_handle >= 0);

    if (opus_baud_rate == OPUS_BAUD_RATE) {
        return;
    }

    E_DBG("Passthru check");

    /* No bytes at all, increment the loop counter */
    if (rx_buff_lim == 0) {
        if (loop_cnt++ > 10) {
            /* New baud rate for CGPS board */
            if (opus_baud_rate == 115200) {
                opus_baud_rate = 9600;
            } else if (opus_baud_rate == 9600) {
                /* If we've tried 115200 and 9600, give up and hope it's 
                 * already in MP pass-thru mode */
                opus_baud_rate = OPUS_BAUD_RATE;
            } else {
                opus_baud_rate = 115200;
            }
            E_DBG("_set_core_to_passthru had %d iterations, try %d baud (0x%x)",
                    loop_cnt, opus_baud_rate, fcm_handle);

            /* Must set state back to "AT" state to change baud rate */

            res = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_AT);
            if (res != OK) {
                ERR_LOG(adl_fcmSwitchV24State, res);
                notify_err(GPS_ERR_FCM_DATA);
            }
            rx_buff_clear();
            loop_cnt = 0;
            return;
        }
        /* Wait for bytes in */
        passthru_timer = adl_tmrSubscribe (FALSE, 2, ADL_TMR_TYPE_100MS, 
                _set_core_to_passthru_timer);
        if (!(passthru_timer > 0)) {
            ERR_LOG(adl_tmrSubscribe, passthru_timer);
            notify_err(GPS_ERR_GPIO_CONF);
        }
        return;
    }

    E_DBG("_set_core_to_passthru has %d bytes",(int)rx_buff_lim);

    /* Look for string (at 115200) indicating Opus III Bootup routine, ready to
     * switch to passthru: "ERIDE_TM_READY" */
    if (strstr((char *)rx_buff, "ERIDE_TM_READY") != NULL) {
        U16 sent;
        U16 len;
        E_DBG("_set_core_to_passthru found ERIDE_TM_READY, send: %s", 
                OPUS_PASSTHRU_STRING);

        len = (U16) e_strlen(OPUS_PASSTHRU_STRING);
        sent = 0;
        while (TRUE) {
            sent += cb_put_gps_data(((U8 *) OPUS_PASSTHRU_STRING) + sent, 
                                    len-sent);
            /* FIXME: must wait to comply with cb_put_gps_data semantics, but
             * this may hang indefinitely... should use a timer */
            if (sent >= len) {
                break;
            } else {
                cb_sleep(100);
            }
        }
        opus_baud_rate = OPUS_BAUD_RATE;

        if (fcm_handle < 0) {
            E_FATAL("invalid fcm_handle");
            return;
        }

        res = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_AT);
        if (res != OK) {
            ERR_LOG(adl_fcmSwitchV24State, res);
            notify_err(GPS_ERR_FCM_DATA);
        }
        rx_buff_clear();
        loop_cnt = 0;
        return;
    }

    /* Look for string (at 9600) indicating Opus III acting as PVT, 
     * outputting NMEA strings */
    if (strstr((char *)rx_buff, "$GPRMC") != NULL) {
        U8 tempBuf[86];
        U16 sent;
        U16 len;
        E_DBG("found $GPRMC - NMEA mode");

        /*@-bufferoverflowhigh@*/
        wm_sprintf((char *)tempBuf, OPUS_PASSTHRU_NMEA);
        /*@+bufferoverflowhigh@*/

        erNmeaAddChecksum((signed char *)tempBuf);
        len = (U16) e_strlen((char *)tempBuf);
        sent = 0;
        while (TRUE) {
            sent += cb_put_gps_data(tempBuf + sent, len-sent);
            /* FIXME: must wait to comply with cb_put_gps_data semantics, but
             * this may hang indefinitely... should use a timer */
            if (sent >= len) {
                break;
            } else {
                cb_sleep(100);
            }
        }

        if (fcm_handle < 0) {
            E_FATAL("invalid fcm_handle");
            return;
        }

        /* Default baud rate for CGPS board */
        opus_baud_rate = OPUS_BAUD_RATE;
        /* Must set state back to "AT" state to change baud rate */
        res = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_AT);
        if (res != OK) {
            ERR_LOG(adl_fcmSwitchV24State, res);
            notify_err(GPS_ERR_FCM_DATA);
        }
        /* Clear out any old OPUS packets */
        rx_buff_clear();
        loop_cnt = 0;
        return;
    }

    if (loop_cnt++ > 10) {
        /* New baud rate for CGPS board */
        if (opus_baud_rate == 115200) {
            opus_baud_rate = 9600;
        } else if (opus_baud_rate == 9600) {
            /* If we've tried 115200 and 9600, give up and hope it's already 
             * in MP pass-thru mode */
            opus_baud_rate = OPUS_BAUD_RATE;
        } else {
            opus_baud_rate = 115200;
        }
        E_DBG("_set_core_to_passthru had %d iterations, try %d baud (0x%x)", 
                loop_cnt, opus_baud_rate, fcm_handle);

        if (fcm_handle < 0) {
            E_FATAL("invalid fcm_handle");
            return;
        }

        /* Must set state back to "AT" state to change baud rate */
        res = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_AT);
        if (res != OK) {
            ERR_LOG(adl_fcmSwitchV24State, res);
            notify_err(GPS_ERR_FCM_DATA);
        }
        /* Clear out any old OPUS packets */
        rx_buff_clear();
        loop_cnt = 0;
        return;
    }

    /* Clear out any old OPUS packets */
    rx_buff_clear();
    passthru_timer = adl_tmrSubscribe (FALSE, 1, ADL_TMR_TYPE_100MS, 
            _set_core_to_passthru_timer);
    if (!(passthru_timer > 0)) {
        ERR_LOG(adl_tmrSubscribe, passthru_timer);
        notify_err(GPS_ERR_GPIO_CONF);
    }

    return;
}

e_gps_position *e_gps_get_current_fix(void)
{
    return &curr_pos;
}

e_gps_pvt_handler e_gps_get_pvt_handler(void)
{
    return gps_pvt_handler;
}

void e_gps_unregister_pvt_handler(void)
{
    gps_pvt_handler = NULL;
}

void e_gps_register_pvt_handler(e_gps_pvt_handler handler)
{
    e_assert(gps_pvt_handler == NULL);
    gps_pvt_handler = handler;
}

e_gps_err_handler e_gps_get_err_handler(void)
{
    return gps_err_handler;
}

void e_gps_unregister_err_handler(void)
{
    gps_err_handler = NULL;
}

void e_gps_register_err_handler(e_gps_err_handler handler) 
{
    e_assert(gps_err_handler == NULL);
    gps_err_handler = handler;
}

void e_gps_stop(e_gps_stop_handler handler)
{
    s32 res = -1;
    s8 res2 = -1;

    if (E_GPS_STOPPING == state) {
        return;
    }

    E_DBG("Stopping GPS");

    erGpsOff();

    /* if we got new NV_DATA, also save at shutdown */
    if (_wrote_new_nvdata) {
        _wrote_new_nvdata = FALSE;
        (void) save_nv_data();
    }

    gps_stop_handler = handler;
    _switch_state(E_GPS_STOPPING);

    if (send_buffer_retry_timer != NULL && send_buffer_retry_timer > 0) {
	    res = adl_tmrUnSubscribe(send_buffer_retry_timer, send_buffer_retry, 
                ADL_TMR_TYPE_TICK);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        }
        send_buffer_retry_timer = NULL;
    }
    if (check_gps_data_timer != NULL && check_gps_data_timer > 0) {
	    res = adl_tmrUnSubscribe(check_gps_data_timer, check_gps_data, 
                ADL_TMR_TYPE_TICK);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        }
        check_gps_data_timer = NULL;
    }
    if (do_gps_start_timer != NULL && do_gps_start_timer > 0) {
	    res = adl_tmrUnSubscribe(do_gps_start_timer, do_gps_start, 
                ADL_TMR_TYPE_100MS);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        }
        do_gps_start_timer = NULL;
    }
    if (gpio_init_timer != NULL && gpio_init_timer > 0) {
	    res = adl_tmrUnSubscribe(gpio_init_timer, gpio_init, 
                ADL_TMR_TYPE_100MS);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        }
        gpio_init_timer = NULL;
    }
    if (gpio_handle >= 0 ) {
        res = adl_ioUnsubscribe(gpio_handle);
        if (res != OK) {
            ERR_LOG(adl_ioUnsubscribe, gpio_handle);
        }
        gpio_handle = -1;
    }

#ifdef WM_MOBIPOWER
    gpio_init_state = 0;
#endif

    if (fcm_handle >= 0) {
        /* switch back to AT state */
		res2 = adl_fcmSwitchV24State(fcm_handle, ADL_FCM_V24_STATE_AT);
        if (res2 != OK) {
            ERR_LOG(adl_fcmSwitchV24State, res);
            notify_err(GPS_ERR_FCM_DATA);
        }
    } else {
        _switch_state(E_GPS_INIT);
        if (gps_stop_handler) {
            gps_stop_handler();
        }
    }
}

s32 e_gps_start(adl_port_e uart)
{
    s32 rc = -1;
    s8 res = 0;

    E_STACK("e_gps_start enter");

    state = E_GPS_INIT;

    _wrote_new_nvdata = FALSE;

    (void) e_mem_set(&curr_pos, 0, sizeof(curr_pos));

    /* Open UART */
    /*
     * WMFM: Multiflow Management, 0 = managing physical ports, 1 = open, [1|2] == port
     */
    if(ADL_PORT_UART1 == uart) {
    	res = adl_atCmdCreate("AT+WMFM=0,1,1", ADL_PORT_NONE, _uart_open_response_handler, NULL );
        if (res != OK) {
            ERR_LOG(adl_atCmdCreate, res);
            goto exit;
        }
    } else if (ADL_PORT_UART2 == uart) {
    	res = adl_atCmdCreate("AT+WMFM=0,1,2", ADL_PORT_NONE, _uart_open_response_handler, NULL );
        if (res != OK) {
            ERR_LOG(adl_atCmdCreate, res);
            goto exit;
        }
    } else {
        E_DBG("Unsupported UART");
        goto exit;
    }

    gps_uart = uart;

    /* Configure the UART */
#ifdef WM_FASTRACK
    opus_baud_rate = 57600;
#elif defined(WM_MOBIPOWER)
    opus_baud_rate = 115200;
#endif

    if(_configure_gps_port() < 0) {
        goto exit;
    }

    rc = 0;

exit:
    E_STACK("e_gps_start exit");
    return rc;
}
