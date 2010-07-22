#include "ok.h"

int layer2 = -1;
int protocol = -1;
int present = 0;

static struct rfid_reader_handle *rh;
static struct rfid_layer2_handle *l2h;
static struct rfid_protocol_handle *ph;

static const char * hexdump(const void *data, unsigned int len)
{
	static char string[2048];
	unsigned char *d = (unsigned char *) data;
	unsigned int i, left, llen = len;

	string[0] = '\0';
	left = sizeof(string);
	for (i = 0; llen--; i += 3) {
		if (i >= sizeof(string) -4)
			break;
		snprintf(string+i, 4, " %02x", *d++);
	} return string;
	
	if (i >= sizeof(string) -2)
		return string;
	snprintf(string+i, 2, " ");
	i++; llen = len;
	
	d = (unsigned char *) data;
	for (; llen--; i += 1) {
		if (i >= sizeof(string) -2)
			break;
		snprintf(string+i, 2, "%c", isprint(*d) ? *d : '.');
		d++;
	}
	return string;
}

static int reader_init(void) 
{
	printf("opening reader handle CM5x21\n");
	rh = rfid_reader_open(NULL, RFID_READER_CM5121);
	if (!rh) {
		fprintf(stderr, "No Omnikey Cardman 5x21 found\n");
		return -1;
	}
	return 0;
}

static int l2_init(int layer2)
{
	int rc;

	printf("opening layer2 handle\n");
	l2h = rfid_layer2_init(rh, layer2);
	if (!l2h) {
		fprintf(stderr, "error during layer2(%d)_init (0=14a,1=14b,3=15)\n",layer2);
		return -1;
	}

	printf("running layer2 anticol(_open)\n");
	rc = rfid_layer2_open(l2h);
	if (rc < 0) {
		fprintf(stderr, "error during layer2_open\n");
		return rc;
	}

	return 0;
}

static int l3_init(int protocol)
{
	printf("running layer3 (ats)\n");
	ph = rfid_protocol_init(l2h, protocol);
	if (!ph) {
		fprintf(stderr, "error during protocol_init\n");
		return -1;
	}
	if (rfid_protocol_open(ph) < 0) {
		fprintf(stderr, "error during protocol_open\n");
		return -1;
	}

	printf("we now have layer3 up and running\n");

	return 0;
}

/* scan for devices */
RESPONSECODE initLib()
{
	RESPONSECODE rv;
	
	rfid_init();
	if (reader_init() < 0)
	{
		Log1(PCSC_LOG_CRITICAL, "reader_init() failed");
		return IFD_COMMUNICATION_ERROR;
	}
	layer2 = RFID_LAYER2_ISO14443A;
	protocol = RFID_PROTOCOL_TCL;
/*
	if (adapter_id < 0)
	{
		fprintf(stderr, "couldn't get adapter: error code %d: %s\n",
				errno, strerror(errno));
		return IFD_COMMUNICATION_ERROR;
	}
	
	scan_sock = hci_open_dev(adapter_id);
	if (scan_sock < 0)
	{
		Log1(PCSC_LOG_CRITICAL, "opening socket failed");
		return IFD_COMMUNICATION_ERROR;
	}
	
	len = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	devices = (inquiry_info*) malloc(max_rsp * sizeof(inquiry_info));
	
	// scan
	num_rsp = hci_inquiry(adapter_id, len, max_rsp, NULL, &devices, flags);
	
	if (num_rsp < 0)
	{
		Log1(PCSC_LOG_CRITICAL, "hci_inquiry failed");
		return IFD_COMMUNICATION_ERROR;
	}
	
	if (num_rsp == 0)
	{
		Log1(PCSC_LOG_CRITICAL, "no devices found");
		return IFD_COMMUNICATION_ERROR;
	}
	
	// try to find the service on every device
	for (i = 0; i < num_rsp; ++i)
	{
		ba2str(&(devices + i)->bdaddr, addr);
		memset(name, 0, sizeof(name));
		if (0 != hci_read_remote_name(scan_sock, &(devices + i)->bdaddr,
									  sizeof(name), name, 0))
		{
			strcpy(name, "[unknown]");
		}
		printf("trying: %s %s\n", addr, name);
		
		// return first found
		if (getService(devices->bdaddr) == IFD_SUCCESS)
		{
			return IFD_SUCCESS;
		}
	}*/
	return IFD_SUCCESS;
}

void closeLib()
{
	RESPONSECODE rv;

	if (present == 1)
	{
		rfid_protocol_close(ph);
		rfid_layer2_close(l2h);
	}

	present = 0;
	rfid_reader_close(rh);
}

/* wrap data, transmit to the device and get answer */
RESPONSECODE sendData(PUCHAR TxBuffer, DWORD TxLength, 
				 PUCHAR RxBuffer, PDWORD RxLength, int wait)
{
	RESPONSECODE rc;
	
	if (present == 0)
	{
		return IFD_COMMUNICATION_ERROR;
	}
	
	unsigned char apdu[TxLength];
	unsigned char buffer[2000];
	unsigned int i, j, response_length, n = 0;
	int sLen = TxLength;
	
	for (i = 0; i < TxLength; ++i)
	{
		apdu[i] = *(TxBuffer + i);
	}

	Log2(PCSC_LOG_DEBUG, "preparing to write %d data bytes", TxLength);
	printf(">> %s \n", hexdump(TxBuffer, TxLength));
	int rv;

	rv = rfid_protocol_transceive(ph, TxBuffer, TxLength, RxBuffer, &response_length, 0, 0);
	if (rv < 0)
	{
		Log2(PCSC_LOG_ERROR, "Error from transceive, returned %d\n", rv);
		present = 0;
		rfid_protocol_close(ph);
		rfid_layer2_close(l2h);
		rfid_reader_close(rh);
		return IFD_COMMUNICATION_ERROR;
	}

	Log2(PCSC_LOG_DEBUG, "wrote %d data bytes", TxLength);		
	Log2(PCSC_LOG_DEBUG, "got %d response bytes", response_length);
		
    if (response_length > 0)
    {
		*RxLength = response_length;
		
		//for (j = 0; j < response_length; ++j)
		//{
		//	*(RxBuffer + j) = buffer[j];
		//}
		rc = IFD_SUCCESS;
	    
	}
	else
	{
	    Log1(PCSC_LOG_ERROR, "no response, meaning eof, reader not usable anymore\n");
		present = 0;
		rfid_protocol_close(ph);
		rfid_layer2_close(l2h);
		rfid_reader_close(rh);
	    rc = IFD_COMMUNICATION_ERROR;
	}
	return rc;
}

/* get the uid from the device and return it as an atr */
void readUID(PDWORD Length, PUCHAR Value)
{
	unsigned int size;
	unsigned int size_len = sizeof(size);
	char *data;
	unsigned int data_len;

	if (present == 0)
	{
		*Length = 0;
		return;
	}
	if (rfid_protocol_getopt(ph, RFID_OPT_PROTO_SIZE, &size, &size_len) == 0)
			printf("Size: %u bytes\n", size);
	size_len = sizeof(size);
	size = 0;
	if (rfid_protocol_getopt(ph, RFID_OPT_P_TCL_ATS_LEN, &size, &size_len) == 0)
	{
		data_len = size + 1;
		data = malloc(data_len);
		if (data)
		{
			if (rfid_protocol_getopt(ph, RFID_OPT_P_TCL_ATS, data, &data_len) == 0)
			{
				Log2(PCSC_LOG_DEBUG, "got %d bytes of ATS", data_len);

				int i ,j;

				// construct standart contactless ATR
				*(Value + 0) = 0x3B; //TS direct convention
				*(Value + 1) = 0x88; //T0 TD1 available, 8 historical bytes
				*(Value + 2) = 0x80; //TD1 TD2 follows, protocol T0
				*(Value + 3) = 0x01; //TD2 no Tx3, protocol T1
				j = 4;
				char crc = 0x88 ^ 0x80 ^ 0x01;
				for (i = 5; i < data_len; ++i)
				{
					*(Value + i - 5 + j) = data[i];
					crc ^= data[i];
				}
				*(Value + i + j) = crc;
				*Length = data_len - 5 + j + 1;

				return;
			}
		}
	}	

	*Length = 0;	
}

/* answer to the pcscd polling */
int readPresence()
{
	if (present == 1)
	{
		return present;
	}
	if (l2_init(layer2) < 0) {
		return 0;
	}

	if (l3_init(protocol) < 0) {
		return 0;
	}

	present = 1;
	
	return 1;
}
