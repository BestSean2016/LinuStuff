#include "readdisk.h"
#include "mypipe.h"
#include "netif.h"

static int _get_hd_sn_1(const char* szDevName, char* szSN, int nLimit);
static int _get_hd_sn_2(const char* szDevName, char* szSN, int nLimit);

void print_hard_disk_lisk(void)
{
    PedDevice* dev = 0;
    ped_device_probe_all();
    while((dev = ped_device_get_next(dev))) {
        printf("Device is %s, model is %s\n", dev->path, dev->model);
    }
}

int get_first_hd(char* devName, int len)
{
    PedDevice* dev = 0;
    ped_device_probe_all();
    dev = ped_device_get_next(dev);
    if (dev) {
        strncpy(devName, dev->path, len);
        return (0);
    }
    
    return (-1);
}


static int _get_hd_sn_1(const char* szDevName, char* szSN, int nLimit)
{
    struct hd_driveid id;
    int  nRtn = -1;
    int  fd = open(szDevName, O_RDONLY|O_NONBLOCK); 
    if (fd < 0) {
        perror(szDevName);
        return nRtn;
    }
    
    while(1) {
        if(!ioctl(fd, HDIO_GET_IDENTITY, &id)) {
            char* p = (char*)id.serial_no;
            int len = 0;
            
            if (nLimit < (int)strlen(p)) {
                nRtn = -1;
                break;
            }
            
            while(*p) {
                if (!isspace(*p) && len < nLimit - 2)
                    szSN[len++] = *p;
                ++p;
            }
            
            szSN[len] = 0;
            nRtn = 0;
        }
        
        break;
    }    
    close(fd);
    return nRtn;
}

int  get_first_hd_via_pipe(char* devName, int len) {
    int found = 0;
    MYPIPE_OPEN("fdisk -l", "r")
    MYPIPE_READ_LINE
        //Disk /dev/sda: 1000.2 GB, 1000204886016 bytes, 1953525168 sectors
        if (!found && !strncmp(the_pipe_buffer, "Disk ", 5)) {
            char* p = strchr(the_pipe_buffer, ':');
            if (p) {
                *p = 0;
                strncpy(devName, (char*)the_pipe_buffer + 5, len);
                *p = ':';
                found = 1;
            } else {
                the_pipe_result = -2;
            }
        }
    MYPIPE_CLOSE
    return (found) ? 0 : the_pipe_result;
}


int  get_first_hdsn_via_pipe(const char* szDevName, char* szSN, int nLimit) {
    int found = 0;

    char command[128];
    snprintf(command, 128, "hdparm -i %s", szDevName);
    
    MYPIPE_OPEN(command, "r")
    MYPIPE_READ_LINE
    // Model=WDC WD10EADS-00M2B0, FwRev=01.00A01, SerialNo=WD-WCAV56962982
    if (!found) {
        char *p = strstr(the_pipe_buffer, "SerialNo=");
        if (p) {
            p += strlen("SerialNo=");
            if (strlen(p) < 1)
                continue;   //read nect line

            if (nLimit < strlen(p))
                the_pipe_result = -6;
            
            //we got the serial number
            //p1 is SerialNo=WD-WCAV56962982\n
            found = 1;
            int len = 0;
            while(*p) {
                if (!isspace(*p) && len < nLimit - 2)
                    szSN[len++] = *p;
                ++p;
            }
            szSN[len] = 0;
        }
    }
    MYPIPE_CLOSE
    MYPIPE_RETURN_PIPE_RESULT
    return (found) ? 0 : the_pipe_result;
}



#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>


#define SCSI_TIMEOUT 5000 /* ms */

static int scsi_io(int fd, unsigned char *cdb, unsigned char cdb_size, int xfer_dir,
            unsigned char *data, unsigned int *data_size,
            unsigned char *sense, unsigned int *sense_len)
{
    sg_io_hdr_t io_hdr;

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';

    /* CDB */
    io_hdr.cmdp = cdb;
    io_hdr.cmd_len = cdb_size;

    /* Where to store the sense_data, if there was an error */
    io_hdr.sbp = sense;
    io_hdr.mx_sb_len = *sense_len;
    *sense_len=0;

    /* Transfer direction, either in or out. Linux does not yet
 *        support bidirectional SCSI transfers ?
 *             */
    io_hdr.dxfer_direction = xfer_dir;

    /* Where to store the DATA IN/OUT from the device and how big the
 *        buffer is
 *             */
    io_hdr.dxferp = data;
    io_hdr.dxfer_len = *data_size;

    /* SCSI timeout in ms */
    io_hdr.timeout = SCSI_TIMEOUT;


    if(ioctl(fd, SG_IO, &io_hdr) < 0){
        //perror("SG_IO ioctl failed");
        return -1;
    }

    /* now for the error processing */
    if((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK){
        if(io_hdr.sb_len_wr > 0){
            *sense_len=io_hdr.sb_len_wr;
            return 0;
        }
    }
    if(io_hdr.masked_status){
        //printf("status=0x%x\n", io_hdr.status);
        //printf("masked_status=0x%x\n", io_hdr.masked_status);
        return -2;
    }
    if(io_hdr.host_status){
        //printf("host_status=0x%x\n", io_hdr.host_status);
        return -3;
    }
    if(io_hdr.driver_status){
        //printf("driver_status=0x%x\n", io_hdr.driver_status);
        return -4;
    }
    return 0;
}

static int scsi_inquiry_unit_serial_number(int fd, char *sn, size_t maxlen)
{
    unsigned char cdb[]={0x12,0x01,0x80,0,0,0};

    unsigned int data_size=0x00ff;
    unsigned char data[data_size];

    unsigned int sense_len=32;
    unsigned char sense[sense_len];

    int res, pl, i, l;

    cdb[3]=(data_size>>8)&0xff;
    cdb[4]=data_size&0xff;


    //printf("INQUIRY Unit Serial Number:\n");

    res = scsi_io(fd, cdb, sizeof(cdb), SG_DXFER_FROM_DEV, data, &data_size, sense, &sense_len);
    if(res){
        //printf("SCSI_IO failed\n");
        return -1;
    }
    if(sense_len){
        return -1;
    }

    /* Page Length */
    pl = data[3];

    /* Unit Serial Number */
    //printf("Unit Serial Number:");
    for(i=4, l = 0; i<(pl+4) && l < maxlen; i++)
        //printf("%c",data[i]&0xff);printf("\n");
        if (!isspace(data[i]))
            sn[l++] = data[i] & 0xff;
        
    return 0;
}

static int open_scsi_device(const char *dev)
{
    int fd, vers;

    if((fd=open(dev, O_RDWR))<0){
        //printf("ERROR could not open device %s\n", dev);
        return -1;
    }
    if ((ioctl(fd, SG_GET_VERSION_NUM, &vers) < 0) || (vers < 30000)) {
        //printf("/dev is not an sg device, or old sg driver\n");
        close(fd);
        return -1;
    }

    return fd;
}


static int _get_hd_sn_2(const char* szDevName, char* szSN, int nLimit) {
    int fd;
    
    fd = open_scsi_device(szDevName);
    if (fd < 0) return (-1);

    if (scsi_inquiry_unit_serial_number(fd, szSN, nLimit))
        return (-2);

    return (0);
}

int get_first_hdsn(char* szSN, int nLimit)
{
    char szDevPath[256];

    if (get_first_hd(szDevPath, sizeof(szDevPath)))
        return (-1);

    if (_get_hd_sn_1(szDevPath, szSN, nLimit))
        if (_get_hd_sn_2(szDevPath, szSN, nLimit))
            return first_if_mac(szSN, nLimit);
    
    return (0);
}

