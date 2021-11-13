#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_connect.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include <queue.h>
#include <oc_mqtt_al.h>
#include <oc_mqtt_profile.h>
#include "E53_SF1.h"
#include "E53_IA1.h"
#include <dtls_al.h>
#include <mqtt_al.h>

#define CONFIG_WIFI_SSID          "HUAWEI-3DTWJU"                            //修改为自己的WiFi 热点账号

#define CONFIG_WIFI_PWD           "wenxuan102"                        //修改为自己的WiFi 热点密码

#define CONFIG_APP_SERVERIP       "121.36.42.100"

#define CONFIG_APP_SERVERPORT     "1883"

#define CONFIG_APP_DEVICEID       "61839c96d0a1830285b994f6_20211104"      //替换为注册设备后生成的deviceid

#define CONFIG_APP_DEVICEPWD      "12345678"                                   //替换为注册设备后生成的密钥

#define CONFIG_APP_LIFETIME       60     ///< seconds

#define CONFIG_QUEUE_TIMEOUT      (5*1000)

#define MSGQUEUE_OBJECTS 16 // number of Message Queue Objects

typedef enum
{
    en_msg_cmd = 0,
    en_msg_report,
}en_msg_type_t;

typedef struct
{
    char *request_id;
    char *payload;
} cmd_t;

typedef struct
{
    char smokevalue[5];
    int lum;
    int temp;
    int hum;

} report_t;

typedef struct
{
    en_msg_type_t msg_type;
    union
    {
        cmd_t cmd;
        report_t report;
    } msg;
} app_msg_t;

typedef struct
{
    queue_t         *app_msg;
    int             connected;
    int             beep;
    int             door;
} app_cb_t;
static app_cb_t g_app_cb;

static void deal_report_msg(report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t smoke_value;
    oc_mqtt_profile_kv_t beep;
    oc_mqtt_profile_kv_t door;
    oc_mqtt_profile_kv_t temperature;
    oc_mqtt_profile_kv_t humidity;
    oc_mqtt_profile_kv_t luminance;

    if(g_app_cb.connected != 1){
        return;
    }

    service.event_time = NULL;
    service.service_id = "Smoke";
    service.service_property = &smoke_value;
    service.nxt = NULL;

    smoke_value.key = "Smoke_Value";
    smoke_value.value = &report->smokevalue;
    smoke_value.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    smoke_value.nxt = &beep;

    beep.key = "BeepStatus";
    beep.value = g_app_cb.beep ? "ON" : "OFF";
    beep.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    beep.nxt = &door;

    door.key = "DoorStatus";
    door.value = g_app_cb.door ? "ON" : "OFF";
    door.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    door.nxt = &temperature;

    temperature.key = "Temperature";
    temperature.value = &report->temp;
    temperature.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    temperature.nxt = &humidity;

    humidity.key = "Humidity";
    humidity.value = &report->hum;
    humidity.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    humidity.nxt = &luminance;

    luminance.key = "Luminance";
    luminance.value = &report->lum;
    luminance.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    luminance.nxt = NULL;

    oc_mqtt_profile_propertyreport(NULL,&service);
    return;
}

//use this function to push all the message to the buffer
static int msg_rcv_callback(oc_mqtt_profile_msgrcv_t *msg)
{
    int    ret = 0;
    char  *buf;
    int    buf_len;
    app_msg_t *app_msg;

    if((NULL == msg)|| (msg->request_id == NULL) || (msg->type != EN_OC_MQTT_PROFILE_MSG_TYPE_DOWN_COMMANDS)){
        return ret;
    }

    buf_len = sizeof(app_msg_t) + strlen(msg->request_id) + 1 + msg->msg_len + 1;
    buf = malloc(buf_len);
    if(NULL == buf){
        return ret;
    }
    app_msg = (app_msg_t *)buf;
    buf += sizeof(app_msg_t);

    app_msg->msg_type = en_msg_cmd;
    app_msg->msg.cmd.request_id = buf;
    buf_len = strlen(msg->request_id);
    buf += buf_len + 1;
    memcpy(app_msg->msg.cmd.request_id, msg->request_id, buf_len);
    app_msg->msg.cmd.request_id[buf_len] = '\0';

    buf_len = msg->msg_len;
    app_msg->msg.cmd.payload = buf;
    memcpy(app_msg->msg.cmd.payload, msg->msg, buf_len);
    app_msg->msg.cmd.payload[buf_len] = '\0';

    ret = queue_push(g_app_cb.app_msg,app_msg,10);
    if(ret != 0){
        free(app_msg);
    }

    return ret;
}

///< COMMAND DEAL
#include <cJSON.h>
static void deal_cmd_msg(cmd_t *cmd)
{
    cJSON *obj_root;
    cJSON *obj_cmdname;
    cJSON *obj_paras;
    cJSON *obj_para;

    int cmdret = 1;
    oc_mqtt_profile_cmdresp_t cmdresp;
    obj_root = cJSON_Parse(cmd->payload);
    if (NULL == obj_root)
    {
        goto EXIT_JSONPARSE;
    }

    obj_cmdname = cJSON_GetObjectItem(obj_root, "command_name");
    if (NULL == obj_cmdname)
    {
        goto EXIT_CMDOBJ;
    }
    if (0 == strcmp(cJSON_GetStringValue(obj_cmdname), "Smoke_Control_Beep"))
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (NULL == obj_paras)
        {
            goto EXIT_OBJPARAS;
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Beep");
        if (NULL == obj_para)
        {
            goto EXIT_OBJPARA;
        }
        ///< operate the LED here
        if (0 == strcmp(cJSON_GetStringValue(obj_para), "ON"))
        {
            g_app_cb.beep = 1;
            Beep_StatusSet(ON); 
            printf("Beep On!\r\n");
        }
        else
        {
            g_app_cb.beep = 0;
            Beep_StatusSet(OFF);
            printf("Beep Off!\r\n");
        }
        cmdret = 0;
    }

    if (0 == strcmp(cJSON_GetStringValue(obj_cmdname), "Door_Control"))
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (NULL == obj_paras)
        {
            goto EXIT_OBJPARAS;
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Door");
        if (NULL == obj_para)
        {
            goto EXIT_OBJPARA;
        }
        ///< operate the LED here
        if (0 == strcmp(cJSON_GetStringValue(obj_para), "ON"))
        {
            g_app_cb.door = 1;
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);
            printf("Door On!\r\n");
        }
        else
        {
            g_app_cb.door = 0;
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);
            printf("Door Off!\r\n");
        }
        cmdret = 0;
    }
    

EXIT_OBJPARA:
EXIT_OBJPARAS:
EXIT_CMDOBJ:
    cJSON_Delete(obj_root);
EXIT_JSONPARSE:
    ///< do the response
    cmdresp.paras = NULL;
    cmdresp.request_id = cmd->request_id;
    cmdresp.ret_code = cmdret;
    cmdresp.ret_name = NULL;
    (void)oc_mqtt_profile_cmdresp(NULL, &cmdresp);
    return;
}


static int task_main_entry(void)
{
    app_msg_t *app_msg;
    uint32_t ret ;

    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);
    dtls_al_init();
    mqtt_al_init();
    oc_mqtt_init();
    
    g_app_cb.app_msg = queue_create("queue_rcvmsg",10,1);
    if(NULL ==  g_app_cb.app_msg){
        printf("Create receive msg queue failed");
        
    }
    oc_mqtt_profile_connect_t  connect_para;
    (void) memset( &connect_para, 0, sizeof(connect_para));

    connect_para.boostrap =      0;
    connect_para.device_id =     CONFIG_APP_DEVICEID;
    connect_para.device_passwd = CONFIG_APP_DEVICEPWD;
    connect_para.server_addr =   CONFIG_APP_SERVERIP;
    connect_para.server_port =   CONFIG_APP_SERVERPORT;
    connect_para.life_time =     CONFIG_APP_LIFETIME;
    connect_para.rcvfunc =       msg_rcv_callback;
    connect_para.security.type = EN_DTLS_AL_SECURITY_TYPE_NONE;
    ret = oc_mqtt_profile_connect(&connect_para);
    if((ret == (int)en_oc_mqtt_err_ok)){
        g_app_cb.connected = 1;
        printf("oc_mqtt_profile_connect succed!\r\n");
    }
    else
    {
        printf("oc_mqtt_profile_connect faild!\r\n");
    }
    while (1)
    {
        app_msg = NULL;
        (void)queue_pop(g_app_cb.app_msg,(void **)&app_msg,0xFFFFFFFF);
        if (NULL != app_msg)
        {
            switch (app_msg->msg_type)
            {
            case en_msg_cmd:
                deal_cmd_msg(&app_msg->msg.cmd);
                break;
            case en_msg_report:
                deal_report_msg(&app_msg->msg.report);
                break;
            default:
                break;
            }
            free(app_msg);
        }
    }
    return 0;
}

static int task_sensor_entry(void)
{
    app_msg_t *app_msg;
    float  ppm;
    Init_E53_SF1();
    /****传感器校准****/
    usleep(1000000);        // 开机1s后进行校准
    MQ2_PPM_Calibration();  // 校准传感器

    E53_IA1_Data_TypeDef data;
    E53_IA1_Init();

    while (1)
    {
        ppm = Get_MQ2_PPM();
        if (ppm > 100) Beep_StatusSet(ON);
        E53_IA1_Read_Data(&data);

        app_msg = malloc(sizeof(app_msg_t));
        printf("Smoke ppm:%.3f\r\n", ppm);
        printf("SENSOR:lum:%.2f temp:%.2f hum:%.2f\r\n", data.Lux, data.Temperature, data.Humidity);

        if (NULL != app_msg)
        {
            app_msg->msg_type = en_msg_report;
            sprintf(app_msg->msg.report.smokevalue,"%.3f",ppm);
            app_msg->msg.report.hum = (int)data.Humidity;
            app_msg->msg.report.lum = (int)data.Lux;
            app_msg->msg.report.temp = (int)data.Temperature;

            if(0 != queue_push(g_app_cb.app_msg,app_msg,CONFIG_QUEUE_TIMEOUT)){
                free(app_msg);
            }
        }
        sleep(3);
    }
    return 0;
}

static void F1_Pressed(char *arg)
{
    (void)arg;
    g_app_cb.door = 1;
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);
}
static void F2_Pressed(char *arg)
{
    (void)arg;
    g_app_cb.door = 0;
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);
}

static void MY_APP(void)
{
    osThreadAttr_t attr;

    GpioInit();

    //初始化LED灯
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_GPIO);

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);

    //初始化F1按键，设置为下降沿触发中断
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1_Pressed, NULL);

    //初始化F2按键，设置为下降沿触发中断
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW, F2_Pressed, NULL);


    attr.name = "task_main_entry";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = 24;

    if (osThreadNew((osThreadFunc_t)task_main_entry, NULL, &attr) == NULL)
    {
        printf("Falied to create task_main_entry!\n");
    }
    attr.stack_size = 4096;
    attr.priority = 25;
    attr.name = "task_sensor_entry";
    if (osThreadNew((osThreadFunc_t)task_sensor_entry, NULL, &attr) == NULL)
    {
        printf("Falied to create task_sensor_entry!\n");
    }
}

APP_FEATURE_INIT(MY_APP);