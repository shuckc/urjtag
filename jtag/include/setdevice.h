#define AUTO_DETECT	0
#define SHARC_21065L	0x327A70CB	//should be the dummy device id

typedef struct
{
	unsigned long deviceID;
	unsigned long flash;
	unsigned short algorithm;
	unsigned short unlock_bypass;
}forced_detection_t;
