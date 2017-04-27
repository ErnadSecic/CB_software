

#ifndef ECOM_H
#define ECOM_H



// POD Control related Commands/Events
#define GET_INPUT    1
#define SET_OUTPUT   2
#define POD_EVENT    3
// Ejector Control related Commands
#define MOVE_PUSHER              4
#define STOP_PUSHER              5
#define TIGHTEN_PUSHER           7
#define TEST_PUSHER              8
#define LOAD_SOFTWARE            9
#define RESTART_SOFTWARE         10
#define CONT_BOX_STARTED         11
#define GET_SOFTWARE_VER         14
#define EJECTOR_CONNECTED        15



// ERNAD
// POD CONTROL Equipment Type:
#define EQ_TYPE_CABINET_DOR               100
#define EQ_TYPE_CABINET_TEMPERATURE       200
#define EQ_TYPE_FALL_SENSOR               300
#define EQ_TYPE_FALL_SENSOR_CALIB         301
#define EQ_TYPE_TILT_SENSOR               400
#define EQ_TYPE_CAB_LIGHT_OR_BUZZOR       500
#define EQ_TYPE_HATCH_SENSOR              501 //3.45 (PS)
#define EQ_TYPE_EJECTOR                   600
#define EQ_TYPE_EJECTOR_SWITCH            601
#define EQ_TYPE_RS485                     700
#define EQ_TYPE_TICKET_READER            1000
#define EQ_TYPE_CONTROL_BOX              2000
#define EQ_TYPE_CONTROL_BOX_SW_VER       2001
#define EQ_TYPE_CONTROL_BOX_SW_LOAD      2002


// Message Index
#define MSG_ID_IDX            0
#define MSG_EQ_TYPE_IDX       4
#define MSG_EQ_NO_IDX         8
#define MSG_COMMAND_IDX       12
#define MSG_VAL_LEN_IDX       14
#define MSG_DATA_IDX          16

#define WILDCARD              0xFFFF

// Message specific indexes (MOVE_PUSHER):
#define  DIRECTION_IDX           16        // 1 byte
#define  LENGTH_1_IDX            17        // 4 bytes
#define  SPEED_1_IDX             21        // 3 bytes
#define  AUXILIARY_EJECTOR_IDX   24        // 2 bytes

// Message specific indexes (TIGHTEN):
#define  AUXILIARY_EJECTOR_IDX_TIGHTEN   16        // 2 bytes

/*
// Message specific indexes (MOVE_PUSHER):
#define  DIRECTION_IDX           MSG_DATA_IDX + 0        // 1 byte
#define  DISTANCE_IDX            MSG_DATA_IDX + 1        // 4 bytes
#define  NO_OF_SECTIONS_IDX      MSG_DATA_IDX + 5        // 1 byte
#define  LENGTH_1_IDX            MSG_DATA_IDX + 6        // 4 bytes
#define  SPEED_1_IDX             MSG_DATA_IDX + 10       // 3 bytes
#define  LENGTH_2_IDX            MSG_DATA_IDX + 13       // 4 bytes
#define  SPEED_2_IDX             MSG_DATA_IDX + 17       // 3 bytes
#define  LENGTH_3_IDX            MSG_DATA_IDX + 20       // 4 bytes
#define  SPEED_3_IDX             MSG_DATA_IDX + 24       // 3 bytes
#define  AUXILIARY_EJECTOR_IDX   MSG_DATA_IDX + 27       // 2 bytes*/



#define EQ_VALUE_GET_HIGH                       "01"
#define EQ_VALUE_GET_LOW                        "00"

// Message code values
#define EQ_STATUS_CMD_OK                        "00"
#define EQ_STATUS_NOT_SUCCESSED                 "01"
#define EQ_STATUS_INVALID_VALUES                "02"
#define EQ_STATUS_INVALID_COMMAND               "03"
#define EQ_STATUS_NA                            "04"

#define EQ_STATUS_EQUIPMENT_NO_ERROR            "05"

#define EQ_STATUS_UNKNOWN                       "10"

#define EQ_EJECTOR_NOT_CONNECTED                "50"
#define EQ_EJECTOR_OVERLOAD                     "51"
#define EQ_CAN_NOT_DETECT_PULSES_LOW_LEVEL      "52"  // stoped
#define EQ_CAN_NOT_DETECT_PULSES_HIGH_LEVEL     "53"  // stoped
#define EQ_UNSTABLE_PULSES                      "54"  // stoped
#define EQ_STATUS_MOTOR_BUSY                    "55"
#define EQ_STATUS_LAMP_BROKEN                   "56"

#define EQ_STATUS_EJ_SWITCH_DISABLE             "60"
#define EQ_STATUS_EJ_SWITCH_FAILED_LIGHT_SWITCH "61" //61 = Failed, didn't find switch light
#define EQ_STATUS_EJ_SWITCH_FAILED_HW_IN_USE    "62" //62 = Failed, HW resources already in use.

#define EQ_STATUS_FALL_BROKEN                   "70"
#define EQ_STATUS_FALL_OFF                      "71"
#define EQ_STATUS_FALL_NOT_FINISHED_INIT        "72"

#define EQ_STATUS_RS485_CHCKSUM_ERROR           "80"
#define EQ_STATUS_RS485_LINGTH_OF_DATA_0        "81"
#define EQ_UNSTABILE_RS485                      "82"

#define EQ_STATUS_OLDVALUE                      "90"


int  ecomGetEquipmentNo(unsigned char *pRequest);
int   ecomGetEquipmentType(unsigned char *pRequest);
int  ecomGetCommand(unsigned char *pRequest);
int   ecomStringLength(unsigned char *pRequest);

// Ernad
void ecomDisassembleReceivedCommand(unsigned char *pRequest, int *pEquipmentType, int *pEquipmentNo, int *pRequestCommand, int *pLengthOfValue, int *pValue);
// Ernad
void ecomBuildEventMessage(unsigned char *pResponse, int pnEquipmentType, int pnEquipmentNo, int pnEventCommand, int pnLengthOdValue, char *pValue, char *pEqStatus);


void ecomBuildAckMsg(unsigned char *pRequest, unsigned char *pResponse, char *pEqStatus);
// Ernad
void ecomBuildResponse(unsigned char *pRequest, unsigned char *pResponse, int nLengthOfValue, char * pValue, char *pEqStatus);

int  ecomValidMoveRequest(int iCommand, unsigned char *pRequest, int *pMotorNo, int *pDirection, int *pSteps, int *pSpeed, int *iAuxilaryEqNo);
int ecomValidOpenLedRequest(unsigned char *pRequest, int *pMotorNo, int *pValue, int *iAuxilaryEqNo);

void ecomBuildOperationCompleteMsg(int nUnitID, int iMotorNo, int iDirection, int iTotalCnt, unsigned char * pEqStatus, unsigned char *pResponse);
void   ecomGetValueString(unsigned char *pRequest, unsigned char *nRS485data);


#endif

